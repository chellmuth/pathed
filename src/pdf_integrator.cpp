#include "pdf_integrator.h"

#include "camera.h"
#include "color.h"
#include "globals.h"
#include "job.h"
#include "monte_carlo.h"
#include "path_tracer.h"
#include "photon_pdf.h"
#include "ray.h"
#include "sample.h"
#include "transform.h"
#include "util.h"
#include "vector.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

static std::string zeroPad(int num, int fillCount)
{
    std::ostringstream stream;
    stream << std::setfill('0') << std::setw(fillCount) << num;
    return stream.str();
}

PDFIntegrator::PDFIntegrator()
{
    m_dataSource1 = std::make_shared<DataSource>();
    // m_dataSource2 = std::make_shared<DataSource>();
    // m_dataSource3 = std::make_shared<DataSource>();
    // m_dataSource4 = std::make_shared<DataSource>();
}

Vector3 PDFIntegrator::sample(const Vector3 &normal, RandomGenerator &random, float *pdf) {
    Vector3 hemisphereSample = UniformSampleHemisphere(random);
    Transform hemisphereToWorld = normalToWorldSpace(normal);
    *pdf = INV_TWO_PI;
    return hemisphereToWorld.apply(hemisphereSample);
}

void PDFIntegrator::createPhotons(
    std::shared_ptr<DataSource> dataSource,
    const Scene &scene,
    RandomGenerator &random
) {
    const int photonSamples = g_job->photonSamples();
    const int photonBounces = g_job->photonBounces();

    for (int i = 0; i < photonSamples; i++) {
        LightSample lightSample = scene.sampleLights(random);

        float pdf;
        Vector3 bounceDirection = sample(lightSample.normal, random, &pdf);

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
                dataSource->points.push_back({
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

            // float invPDF = 1.f / INV_TWO_PI;
            // throughput *= invPDF;
        }
    }
}

void PDFIntegrator::preprocess(const Scene &scene, RandomGenerator &random)
{
    createPhotons(m_dataSource1, scene, random);
    // createPhotons(m_dataSource2, scene, random);
    // createPhotons(m_dataSource3, scene, random);
    // createPhotons(m_dataSource4, scene, random);

    m_KDTree1 = std::make_unique<KDTree>(3, *m_dataSource1, nanoflann::KDTreeSingleIndexAdaptorParams(10));
    m_KDTree1->buildIndex();

    // m_KDTree2 = std::make_unique<KDTree>(3, *m_dataSource2, nanoflann::KDTreeSingleIndexAdaptorParams(10));
    // m_KDTree2->buildIndex();

    // m_KDTree3 = std::make_unique<KDTree>(3, *m_dataSource3, nanoflann::KDTreeSingleIndexAdaptorParams(10));
    // m_KDTree3->buildIndex();

    // m_KDTree4 = std::make_unique<KDTree>(3, *m_dataSource4, nanoflann::KDTreeSingleIndexAdaptorParams(10));
    // m_KDTree4->buildIndex();
}

void PDFIntegrator::run(
    Image &image,
    Scene &scene,
    std::function<void(RenderStatus)> callback,
    bool *quit
) {
    RandomGenerator random;

    {
        printf("Beginning pre-process...\n");
        std::clock_t begin = clock();
        preprocess(scene, random);
        std::clock_t end = clock();
        double elapsedSeconds = double(end - begin) / CLOCKS_PER_SEC;
        printf("Pre-process complete (%0.1fs elapsed)\n", elapsedSeconds);
    }

    const int dataPoints = 30;
    for (int i = 0; i < dataPoints; i ++) {
        createAndSaveDataPoint(image, scene, random, i);
        printf("Finished point %i/%i\n", i + 1, dataPoints);
    }

    *quit = true;
}

Intersection PDFIntegrator::generateIntersection(
    const Scene &scene,
    RandomGenerator &random
) {
    const int width = g_job->width();
    const int height = g_job->height();

    while (true) {
        float x = random.next();
        float y = random.next();

        int row = (int)(y * height);
        int col = (int)(x * width);

        Ray ray = scene.getCamera()->generateRay(row, col);
        Intersection intersection = scene.testIntersect(ray);
        if (intersection.hit) {
            return intersection;
        }
    }
}

void PDFIntegrator::createAndSaveDataPoint(
    Image &image,
    const Scene &scene,
    RandomGenerator &random,
    int pointID
) {
    const int width = g_job->width();
    const int height = g_job->height();

    std::vector<float> radianceLookup(3 * width * height);
    for (int i = 0; i < 3 * width * height; i++) {
        radianceLookup[i] = 0.f;
    }

    Intersection intersection = generateIntersection(scene, random);

    savePhotonBundle(m_dataSource1, m_KDTree1, intersection, pointID, "");
    // savePhotonBundle(m_dataSource2, m_KDTree2, intersection, pointID, "b");
    // savePhotonBundle(m_dataSource3, m_KDTree3, intersection, pointID, "c");
    // savePhotonBundle(m_dataSource4, m_KDTree4, intersection, pointID, "d");

    const int spp = 64;
    for (int i = 0; i < spp; i++) {
        renderPDF(radianceLookup, scene, intersection);

        std::mutex &lock = image.getLock();
        lock.lock();

        for (int row = 0; row < height; row++) {
            for (int col = 0; col < width; col++) {
                int index = 3 * (row * width + col);
                image.set(
                    row,
                    col,
                    radianceLookup[index + 0] / (i + 1),
                    radianceLookup[index + 1] / (i + 1),
                    radianceLookup[index + 2] / (i + 1)
                );
            }
        }

        lock.unlock();

        image.setSpp(i + 1);
    }

    std::ostringstream filenameStream;
    filenameStream << "pdf_" << zeroPad(pointID, 5);
    image.save(filenameStream.str());
}

void PDFIntegrator::savePhotonBundle(
    std::shared_ptr<DataSource> dataSource,
    std::unique_ptr<KDTree> &KDTree,
    const Intersection &intersection,
    int pointID,
    const std::string &suffix
) {
    const int debugSearchCount = g_job->debugSearchCount();
    auto resultIndices = std::make_shared<std::vector<size_t>>(debugSearchCount);
    std::vector<float> outDistanceSquared(debugSearchCount);

    nanoflann::KNNResultSet<float> resultSet(debugSearchCount);
    resultSet.init(resultIndices->data(), outDistanceSquared.data());

    float queryPoint[3] = {
        intersection.point.x(),
        intersection.point.y(),
        intersection.point.z()
    };
    KDTree->findNeighbors(resultSet, queryPoint, nanoflann::SearchParams());

    PhotonPDF photonPDF(
        intersection.point,
        dataSource,
        resultIndices,
        g_job->phiSteps(),
        g_job->thetaSteps()
    );

    std::ostringstream filenameStream;
    filenameStream << "photon-bundle_" << zeroPad(pointID, 5) << suffix;

    Transform worldToNormal = worldSpaceToNormal(
        intersection.normal,
        intersection.wi
    );
    photonPDF.save(filenameStream.str(), worldToNormal);
    // printf("Saved photon bundle\n");
}

void PDFIntegrator::renderPDF(
    std::vector<float> &radianceLookup,
    const Scene &scene,
    const Intersection &intersection
) {
    const int phiSteps = g_job->width();
    const int thetaSteps = g_job->height();

    BounceController bounceController(1, 1);
    PathTracer pathTracer(bounceController);

    RandomGenerator random;

    Transform hemisphereToWorld = normalToWorldSpace(
        intersection.normal,
        intersection.wi
    );

    #pragma omp parallel for
    for (int phiStep = 0; phiStep < phiSteps; phiStep++) {
        for (int thetaStep = 0; thetaStep < thetaSteps; thetaStep++) {
            float phi = M_TWO_PI * (phiStep + random.next()) / phiSteps;
            float theta = (M_PI / 2.f) * (thetaStep + random.next()) / thetaSteps;

            float y = cosf(theta);
            float x = sinf(theta) * cosf(phi);
            float z = sinf(theta) * sinf(phi);

            Vector3 wiHemisphere(x, y, z);
            Vector3 wiWorld = hemisphereToWorld.apply(wiHemisphere);

            Ray ray = Ray(intersection.point, wiWorld);
            const Intersection fisheyeIntersection = scene.testIntersect(ray);
            Color sampleL(0.f);
            if (fisheyeIntersection.hit) {
                Sample sample;

                sampleL = pathTracer.L(
                    fisheyeIntersection,
                    scene,
                    random,
                    sample
                );

                // Color emit = fisheyeIntersection.material->emit();
                // sampleL += emit;
            }

            // radianceLookup[3 * (thetaStep * phiSteps + phiStep) + 0] += sampleL.r();
            // radianceLookup[3 * (thetaStep * phiSteps + phiStep) + 1] += sampleL.g();
            // radianceLookup[3 * (thetaStep * phiSteps + phiStep) + 2] += sampleL.b();

            // const float luminance = sampleL.luminance();
            // radianceLookup[3 * (thetaStep * phiSteps + phiStep) + 0] += luminance;
            // radianceLookup[3 * (thetaStep * phiSteps + phiStep) + 1] += luminance;
            // radianceLookup[3 * (thetaStep * phiSteps + phiStep) + 2] += luminance;

            const float average = sampleL.average();
            radianceLookup[3 * (thetaStep * phiSteps + phiStep) + 0] += average;
            radianceLookup[3 * (thetaStep * phiSteps + phiStep) + 1] += average;
            radianceLookup[3 * (thetaStep * phiSteps + phiStep) + 2] += average;
        }
    }
}
