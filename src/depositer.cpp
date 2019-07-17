#include "depositer.h"

#include "bounce_controller.h"
#include "color.h"
#include "globals.h"
#include "job.h"
#include "light.h"
#include "monte_carlo.h"
#include "path_tracer.h"
#include "photon_pdf.h"
#include "photon_visualization.h"
#include "ray.h"
#include "transform.h"
#include "util.h"
#include "vector.h"

#include "json.hpp"
#include "nanoflann.hpp"
using json = nlohmann::json;

#include <assert.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <math.h>
#include <mutex>
#include <utility>
#include <vector>

static std::mutex lock;

static int maxBounces = 6;

Depositer::Depositer(BounceController bounceController)
    : m_bounceController(bounceController)
{
    m_dataSource = std::make_shared<DataSource>();
    m_eyeDataSource = std::make_shared<DataSource>();
}

void Depositer::preprocess(const Scene &scene, RandomGenerator &random)
{
    createLightPaths(scene, random);
    PhotonVisualization::all(IntersectionHelper::miss, *m_dataSource, "light-photons", 0);

    m_KDTree = std::make_unique<KDTree>(3, *m_dataSource, nanoflann::KDTreeSingleIndexAdaptorParams(10));
    m_KDTree->buildIndex();

    m_eyeTree = std::make_unique<KDTree>(3, *m_eyeDataSource, nanoflann::KDTreeSingleIndexAdaptorParams(10));
    m_eyeTree->buildIndex();
}

Vector3 Depositer::sample(
    const Point3 &point,
    const Vector3 &normal,
    const KDTree *tree,
    int phiSteps,
    int thetaSteps,
    RandomGenerator &random,
    float *pdf
) {
    if (tree == nullptr) {
        Vector3 hemisphereSample = UniformSampleHemisphere(random);
        Transform hemisphereToWorld = normalToWorldSpace(normal);
        *pdf = INV_TWO_PI;
        return hemisphereToWorld.apply(hemisphereSample);
    }

    float queryPoint[3] = {
        point.x(),
        point.y(),
        point.z()
    };

    // const float searchRadius = 0.1f;
    // std::vector<std::pair<size_t, float> > matches;
    // nanoflann::SearchParams params;

    // const size_t matchCount = tree->radiusSearch(queryPoint, searchRadius, matches, params);
    // auto resultIndices = std::make_shared<std::vector<size_t> >(matchCount);
    // for (auto &match : matches) {
    //     resultIndices->push_back(match.first);
    // }

    const int debugSearchCount = g_job->debugSearchCount();
    auto resultIndices = std::make_shared<std::vector<size_t>>(debugSearchCount);
    std::vector<float> outDistanceSquared(debugSearchCount);

    nanoflann::KNNResultSet<float> resultSet(debugSearchCount);
    resultSet.init(resultIndices->data(), outDistanceSquared.data());

    tree->findNeighbors(resultSet, queryPoint, nanoflann::SearchParams());

    PhotonPDF photonPDF(point, m_eyeDataSource, resultIndices, phiSteps, thetaSteps);

    Transform worldToNormal = worldSpaceToNormal(normal);
    Vector3 pdfSample = photonPDF.sample(random, worldToNormal, pdf);

    Transform hemisphereToWorld = normalToWorldSpace(normal);
    return hemisphereToWorld.apply(pdfSample);

    // float pdfCheck = photonPDF.pdf(bounceDirection, worldToNormal);
    // assert(fabsf(pdfCheck - pdf) < 1e-5);
    // if (pdf != INV_TWO_PI) {
    //     printf("%f %f\n", pdf, pdfCheck);
    // }
}

void Depositer::createLightPaths(const Scene &scene, RandomGenerator &random)
{
    const int photonSamples = g_job->photonSamples();
    const int photonBounces = g_job->photonBounces();

    const int phiSteps = g_job->lightPhiSteps();
    const int thetaSteps = g_job->lightThetaSteps();

    for (int i = 0; i < photonSamples; i++) {
        LightSample lightSample = scene.sampleLights(random);

        float pdf;
        Vector3 bounceDirection = sample(
            lightSample.point,
            lightSample.normal,
            m_eyeTree.get(),
            phiSteps,
            thetaSteps,
            random,
            &pdf
        );

        if (bounceDirection.dot(lightSample.normal) < 0.f) {
            assert(false);
            break;
        }

        Ray lightRay(lightSample.point, bounceDirection);

        Color throughput = lightSample.light->getMaterial()->emit();
        for (int bounce = 0; bounce < photonBounces; bounce++) {
            Intersection intersection = scene.testIntersect(lightRay);
            if (!intersection.hit) { break; }

            throughput *= fmaxf(0.f, intersection.wi.dot(intersection.normal * -1.f));
            if (throughput.isBlack()) { break; }

            if (bounce > 0) { // don't guide towards direct lights
                m_dataSource->points.push_back({
                    intersection.point.x(),
                    intersection.point.y(),
                    intersection.point.z(),
                    lightRay.origin(),
                    throughput
                });
            }

            Vector3 hemisphereSample = UniformSampleHemisphere(random);
            Transform hemisphereToWorld = normalToWorldSpace(intersection.normal);
            bounceDirection = hemisphereToWorld.apply(hemisphereSample);
            lightRay = Ray(intersection.point, bounceDirection);

            throughput *= intersection.material->f(intersection, bounceDirection);
        }
    }
}

void Depositer::postwave(const Scene &scene, RandomGenerator &random, int waveCount)
{
    // Create eye tree from previous L calls populating datasource
    m_eyeTree = std::make_unique<KDTree>(3, *m_eyeDataSource, nanoflann::KDTreeSingleIndexAdaptorParams(10));
    m_eyeTree->buildIndex();

    if (waveCount <= 3) {
        PhotonVisualization::all(IntersectionHelper::miss, *m_eyeDataSource, "eye-photons", waveCount);
    }

    // Clear light tree
    m_dataSource->points.clear();

    // Build new light tree from eye tree
    createLightPaths(scene, random);

    if (waveCount <= 3) {
        PhotonVisualization::all(IntersectionHelper::miss, *m_dataSource, "light-photons", waveCount);
    }

    m_KDTree = std::make_unique<KDTree>(3, *m_dataSource, nanoflann::KDTreeSingleIndexAdaptorParams(10));
    m_KDTree->buildIndex();

    // Clear eye source to rebuild in L
    m_eyeDataSource->points.clear();
}

Color Depositer::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    sample.eyePoints.push_back(intersection.point);

    Color modulation = Color(1.f, 1.f, 1.f);

    Color result(0.f);
    if (m_bounceController.checkCounts(1)) {
        result = direct(intersection, modulation, scene, random, sample);
        sample.contributions.push_back(result);
    }

    Intersection lastIntersection = intersection;

    for (int bounce = 2; !m_bounceController.checkDone(bounce); bounce++) {
        Transform hemisphereToWorld = normalToWorldSpace(lastIntersection.normal);

        Point3 intersectionPoint = lastIntersection.point;
        float queryPoint[3] = {
            intersectionPoint.x(),
            intersectionPoint.y(),
            intersectionPoint.z()
        };

        const int debugSearchCount = g_job->debugSearchCount();
        auto resultIndices = std::make_shared<std::vector<size_t>>(debugSearchCount);
        std::vector<float> outDistanceSquared(debugSearchCount);

        nanoflann::KNNResultSet<float> resultSet(debugSearchCount);
        resultSet.init(resultIndices->data(), outDistanceSquared.data());

        m_KDTree->findNeighbors(resultSet, queryPoint, nanoflann::SearchParams());

        // const float searchRadius = 0.1f;
        // std::vector<std::pair<size_t, float> > matches;
        // nanoflann::SearchParams params;

        // const size_t matchCount = m_KDTree->radiusSearch(queryPoint, searchRadius, matches, params);
        // auto resultIndices = std::make_shared<std::vector<size_t> >(matchCount);
        // for (auto &match : matches) {
        //     resultIndices->push_back(match.first);
        // }

        Transform worldToNormal = worldSpaceToNormal(lastIntersection.normal);

        PhotonPDF photonPDF(
            lastIntersection.point,
            m_dataSource,
            resultIndices,
            g_job->phiSteps(),
            g_job->thetaSteps()
        );
        float pdf;
        Vector3 pdfSample = photonPDF.sample(random, worldToNormal, &pdf);

        Vector3 bounceDirection = hemisphereToWorld.apply(pdfSample);

        if (bounceDirection.dot(lastIntersection.normal) < 0.f) {
            assert(false);
            break;
        }

        Ray bounceRay(
            lastIntersection.point,
            bounceDirection
        );

        Intersection bounceIntersection = scene.testIntersect(bounceRay);
        if (!bounceIntersection.hit) { break; }

        if (bounce > 2) {
            DataSource::Point eyeVertex = {
                bounceIntersection.point.x(),
                bounceIntersection.point.y(),
                bounceIntersection.point.z(),
                lastIntersection.point,
                modulation
            };
            lock.lock();
            m_eyeDataSource->points.push_back(eyeVertex);
            lock.unlock();
        }

        sample.eyePoints.push_back(bounceIntersection.point);

        Color f = lastIntersection.material->f(lastIntersection, bounceDirection);
        float invPDF = 1.f / pdf;

        modulation *= f
            * fmaxf(0.f, bounceDirection.dot(lastIntersection.normal))
            * invPDF;
        lastIntersection = bounceIntersection;

        const Color previous = result;

        result += direct(bounceIntersection, modulation, scene, random, sample);

        sample.contributions.push_back(result - previous);
    }

    return result;
}

Color Depositer::direct(
    const Intersection &intersection,
    const Color &modulation,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    Color emit = intersection.material->emit();
    if (!emit.isBlack()) {
        // part of my old logic - if you hit an emitter, don't do direct lighting?
        return Color(0.f, 0.f, 0.f);
    }

    int lightCount = scene.lights().size();
    int lightIndex = (int)floorf(random.next() * lightCount);

    std::shared_ptr<Light> light = scene.lights()[lightIndex];
    SurfaceSample lightSample = light->sample(random);

    Vector3 lightDirection = (lightSample.point - intersection.point).toVector();
    Vector3 wo = lightDirection.normalized();

    if (lightSample.normal.dot(wo) >= 0.f) {
        sample.shadowTests.push_back({
            intersection.point,
            lightSample.point,
            true
        });

        return Color(0.f);
    }

    Ray shadowRay = Ray(intersection.point, wo);
    float lightDistance = lightDirection.length();
    bool occluded = scene.testOcclusion(shadowRay, lightDistance);

    sample.shadowTests.push_back({
        intersection.point,
        lightSample.point,
        occluded
    });

    if (occluded) {
        return Color(0.f);
    }

    float invPDF = lightSample.invPDF * lightCount;

    // DataSource::Point eyeVertex = {
    //     lightSample.point.x(),
    //     lightSample.point.y(),
    //     lightSample.point.z(),
    //     intersection.point,
    //     modulation
    // };
    // lock.lock();
    // m_eyeDataSource->points.push_back(eyeVertex);
    // lock.unlock();

    return modulation * light->biradiance(lightSample, intersection.point)
        * intersection.material->f(intersection, wo)
        * fmaxf(0.f, wo.dot(intersection.normal))
        * invPDF;
}

void Depositer::debug(const Intersection &intersection, const Scene &scene) const
{
    Point3 intersectionPoint = intersection.point;
    float queryPoint[3] = {
        intersectionPoint.x(),
        intersectionPoint.y(),
        intersectionPoint.z()
    };

    const int debugSearchCount = g_job->debugSearchCount();
    auto resultIndices = std::make_shared<std::vector<size_t>>(debugSearchCount);
    std::vector<float> outDistanceSquared(debugSearchCount);

    nanoflann::KNNResultSet<float> resultSet(debugSearchCount);
    resultSet.init(resultIndices->data(), outDistanceSquared.data());

    m_KDTree->findNeighbors(resultSet, queryPoint, nanoflann::SearchParams());

    Transform worldToNormal = worldSpaceToNormal(intersection.normal);
    RandomGenerator random;
    PhotonPDF photonPDF(intersection.point, m_dataSource, resultIndices, g_job->phiSteps(), g_job->thetaSteps());
    float pdf;
    Vector3 wi = photonPDF.sample(random, worldToNormal, &pdf, true);
    Vector3 bounceDirection = wi;
    bounceDirection.debug();

    json j;
    j["QueryPoint"] = { intersectionPoint.x(), intersectionPoint.y(), intersectionPoint.z() };
    j["Results"] = json::array();
    for (int i = 0; i < debugSearchCount; i++) {
        auto &point = m_dataSource->points[resultIndices->at(i)];
        j["Results"].push_back({
            { "point", { point.x, point.y, point.z } },
            { "source", { point.source.x(), point.source.y(), point.source.z() } },
            { "throughput", { point.throughput.r(), point.throughput.g(), point.throughput.b() } },
        });
    }

    std::ofstream jsonFile("live-photons.json");
    jsonFile << j.dump(4) << std::endl;
    std::cout << "Wrote to live-photons.json" << std::endl;

    // std::cout << j.dump(4) << std::endl;

    debug2(intersection, scene);
}

void Depositer::debug2(const Intersection &intersection, const Scene &scene) const
{
    const int phiSteps = 200;
    const int thetaSteps = 200;
    const int spp = 256;

    BounceController bounceController = g_job->bounceController().copyAfterBounce();
    PathTracer integrator(bounceController);
    RandomGenerator random;

    Transform hemisphereToWorld = normalToWorldSpace(intersection.normal);

    json j;
    j["QueryPoint"] = {
        intersection.point.x(),
        intersection.point.y(),
        intersection.point.z()
    };
    j["Steps"] = { { "phi", phiSteps }, { "theta", thetaSteps } };
    j["Gt"] = json::array();

    for (int phiStep = 0; phiStep < phiSteps; phiStep++) {
        for (int thetaStep = 0; thetaStep < thetaSteps; thetaStep++) {
            float phi = M_TWO_PI * phiStep / phiSteps;
            float theta = M_PI * thetaStep / thetaSteps;

            float y = cosf(theta);
            float x = sinf(theta) * cosf(phi);
            float z = sinf(theta) * sinf(phi);

            // Vector3 wiHemisphere(x, y, z);
            // Vector3 wiWorld = hemisphereToWorld.apply(wiHemisphere);

            Vector3 wiWorld = Vector3(x, y, z).normalized();

            Ray ray = Ray(intersection.point, wiWorld);
            const Intersection fisheyeIntersection = scene.testIntersect(ray);
            Color sampleL(0.f);
            if (fisheyeIntersection.hit) {
                for (int i = 0; i < spp; i++) {
                    Sample sample;

                    sampleL += integrator.L(
                        fisheyeIntersection,
                        scene,
                        random,
                        sample
                    ) / spp;

                    if (bounceController.checkCounts(0)) {
                        Color emit = fisheyeIntersection.material->emit();
                        sampleL += emit / spp;
                    }
                }
            }

            // std::cout << "phi: " << phi << " theta: " << theta << std::endl;
            // std::cout << "x: " << x << " y: " << y << " z: " << z << std::endl;
            // std::cout << sampleL << std::endl;

            j["Gt"].push_back({
                { "wi", { x, y, z } },
                { "phiStep", phiStep },
                { "thetaStep", thetaStep },
                { "phi", phi },
                { "theta", theta },
                { "radiance", { sampleL.r(), sampleL.g(), sampleL.b() } },
                { "luminance", { sampleL.luminance() } }
            });
        }
    }

    std::ofstream jsonFile("live-gt.json");
    jsonFile << j.dump(4) << std::endl;
    std::cout << "Wrote to live-gt.json" << std::endl;
}

std::vector<DataSource::Point> Depositer::getPhotons() const {
    return m_dataSource->points;
};

DataSource Depositer::getDataSource() const {
    return *m_dataSource;
}
