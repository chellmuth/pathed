#include "ml_integrator.h"

#include "coordinate.h"
#include "globals.h"
#include "image.h"
#include "job.h"
#include "light.h"
#include "monte_carlo.h"
#include "path_tracer.h"
#include "photon_pdf.h"
#include "ray.h"
#include "transform.h"
#include "util.h"
#include "vector.h"

#include <iomanip>
#include <iostream>

static std::string zeroPad(int num, int fillCount)
{
    std::ostringstream stream;
    stream << std::setfill('0') << std::setw(fillCount) << num;
    return stream.str();
}

MLIntegrator::MLIntegrator(BounceController bounceController)
    : m_bounceController(bounceController)
{
    m_dataSource = std::make_shared<DataSource>();
}

Vector3 MLIntegrator::sample(const Vector3 &normal, RandomGenerator &random, float *pdf)
{
    Vector3 hemisphereSample = UniformSampleHemisphere(random);
    Transform hemisphereToWorld = normalToWorldSpace(normal);
    *pdf = INV_TWO_PI;
    return hemisphereToWorld.apply(hemisphereSample);
}

void MLIntegrator::createPhotons(const Scene &scene, RandomGenerator &random)
{
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

        Color throughput = lightSample.light->emit();
        for (int bounce = 0; bounce < photonBounces; bounce++) {
            Intersection intersection = scene.testIntersect(lightRay);
            if (!intersection.hit) { break; }

            throughput *= fmaxf(0.f, intersection.wo.dot(intersection.normal));
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

            // float invPDF = 1.f / INV_TWO_PI;
            // throughput *= invPDF;
        }
    }
}

void MLIntegrator::preprocess(const Scene &scene, RandomGenerator &random)
{
    createPhotons(scene, random);

    m_KDTree = std::make_unique<KDTree>(3, *m_dataSource, nanoflann::KDTreeSingleIndexAdaptorParams(10));
    m_KDTree->buildIndex();

    if (!m_MLPDF.connectToModel()) {
        printf("We're done here!\n");
        throw "Failure to connect";
    }
}

void MLIntegrator::renderPDF(
    std::vector<float> &radianceLookup,
    const Scene &scene,
    const Intersection &intersection
) const {
    const int phiSteps = g_job->width();
    const int thetaSteps = g_job->height();

    PathTracer pathTracer(g_job->bounceController());

    RandomGenerator random;
    int bounceCount = 2;

    Transform hemisphereToWorld = normalToWorldSpace(
        intersection.normal,
        intersection.wo
    );

    for (int phiStep = 0; phiStep < phiSteps; phiStep++) {
        for (int thetaStep = 0; thetaStep < thetaSteps; thetaStep++) {
            float phi = M_TWO_PI * phiStep / phiSteps;
            float theta = (M_PI / 2.f) * thetaStep / thetaSteps;

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

static int imageIndex = 0;

Vector3 MLIntegrator::nextBounce(const Intersection &intersection, const Scene &scene, float *pdf) const
{
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
    m_KDTree->findNeighbors(resultSet, queryPoint, nanoflann::SearchParams());

    PhotonPDF photonPDF(
        intersection.point,
        m_dataSource,
        resultIndices,
        g_job->phiSteps(),
        g_job->thetaSteps()
    );

    Transform worldToNormal = worldSpaceToNormal(
        intersection.normal,
        intersection.wo
    );
    std::vector<float> photonBundle = photonPDF.asVector(worldToNormal);
    //photonPDF.save("photons", worldToNormal);

    float phi, theta;
    m_MLPDF.sample(&phi, &theta, pdf, photonBundle);

    // std::cout << phi << " " << theta << " " << *pdf << std::endl;

    Vector3 hemisphereSample = sphericalToCartesian(phi, theta);
    // hemisphereSample.debug();

    // Vector3 bounceDirection = worldToNormal.transposed().apply(hemisphereSample);

    // if (imageIndex == 0) {
    //     const int width = g_job->width();
    //     const int height = g_job->height();
    //     Image image(width, height);

    //     std::vector<float> radianceLookup(3 * width * height);
    //     for (int i = 0; i < 3 * width * height; i++) {
    //         radianceLookup[i] = 0.f;
    //     }

    //     const int spp = 64;
    //     for (int i = 0; i < spp; i++) {
    //         renderPDF(radianceLookup, scene, intersection);

    //         std::mutex &lock = image.getLock();
    //         lock.lock();

    //         for (int row = 0; row < height; row++) {
    //             for (int col = 0; col < width; col++) {
    //                 int index = 3 * (row * width + col);
    //                 image.set(
    //                     row,
    //                     col,
    //                     radianceLookup[index + 0] / (i + 1),
    //                     radianceLookup[index + 1] / (i + 1),
    //                     radianceLookup[index + 2] / (i + 1)
    //                 );
    //             }
    //         }

    //         lock.unlock();

    //         image.setSpp(i + 1);
    //     }

    //     std::ostringstream filenameStream;
    //     filenameStream << "pdf_" << zeroPad(imageIndex++, 5);
    //     image.save(filenameStream.str());
    // }

    // if (imageIndex == 1) {
    //     const int width = g_job->width();
    //     const int height = g_job->height();
    //     Image image(width, height);

    //     std::vector<float> radianceLookup(3 * width * height);
    //     for (int i = 0; i < 3 * width * height; i++) {
    //         radianceLookup[i] = 0.f;
    //     }

    //     std::cout << "Start estimating" << std::endl;
    //     m_MLPDF.estimatePDF(radianceLookup, photonBundle);
    //     std::cout << "Finished estimating" << std::endl;

    //     std::mutex &lock = image.getLock();
    //     lock.lock();

    //     for (int row = 0; row < height; row++) {
    //         for (int col = 0; col < width; col++) {
    //             int index = 3 * (row * width + col);
    //             image.set(
    //                 row,
    //                 col,
    //                 radianceLookup[index + 0],
    //                 radianceLookup[index + 1],
    //                 radianceLookup[index + 2]
    //             );
    //         }
    //     }

    //     lock.unlock();

    //     std::ostringstream filenameStream;
    //     filenameStream << "neural_" << zeroPad(imageIndex++, 5);
    //     image.save(filenameStream.str());
    // }

    // return bounceDirection;

    return hemisphereSample;
}

Color MLIntegrator::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    Color result(0.f);
    if (m_bounceController.checkCounts(1)) {
        result = direct(intersection, scene, random);
    }

    Color modulation = Color(1.f);
    Intersection lastIntersection = intersection;

    for (int bounce = 2; !m_bounceController.checkDone(bounce); bounce++) {
        Transform hemisphereToWorld = normalToWorldSpace(
            lastIntersection.normal,
            lastIntersection.wo
        );

        float pdf;
        Vector3 bounceDirection = hemisphereToWorld.apply(nextBounce(intersection, scene, &pdf));

        // Vector3 hemisphereSample = CosineSampleHemisphere(random);
        // float pdf = CosineHemispherePdf(hemisphereSample);
        // Vector3 bounceDirection = hemisphereToWorld.apply(hemisphereSample);

        Ray bounceRay(
            lastIntersection.point,
            bounceDirection
        );

        Intersection bounceIntersection = scene.testIntersect(bounceRay);
        if (!bounceIntersection.hit) { break; }

        Color f = lastIntersection.material->f(lastIntersection, bounceDirection);
        float invPDF = 1.f / pdf;

        modulation *= f
            * fmaxf(0.f, bounceDirection.dot(lastIntersection.normal))
            * invPDF;
        lastIntersection = bounceIntersection;

        if (m_bounceController.checkCounts(bounce)) {
            const Color previous = result;

            result += direct(bounceIntersection, scene, random) * modulation;
        }
    }

    // std::cout << result << std::endl;
    return result;
}

Color MLIntegrator::direct(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random
) const {
    Color emit = intersection.material->emit();
    if (!emit.isBlack()) {
        // part of my old logic - if you hit an emitter, don't do direct lighting?
        return Color(0.f, 0.f, 0.f);
    }

    int lightCount = scene.lights().size();
    int lightIndex = (int)floorf(random.next() * lightCount);

    std::shared_ptr<Light> light = scene.lights()[lightIndex];
    SurfaceSample lightSample = light->sample(intersection, random);

    Vector3 lightDirection = (lightSample.point - intersection.point).toVector();
    Vector3 wo = lightDirection.normalized();

    if (lightSample.normal.dot(wo) >= 0.f) {
        return Color(0.f);
    }

    Ray shadowRay = Ray(intersection.point, wo);
    float lightDistance = lightDirection.length();
    bool occluded = scene.testOcclusion(shadowRay, lightDistance);

    if (occluded) {
        return Color(0.f);
    }

    float invPDF = lightSample.invPDF * lightCount;

    return light->biradiance(lightSample, intersection.point)
        * intersection.material->f(intersection, wo)
        * fmaxf(0.f, wo.dot(intersection.normal))
        * invPDF;
}
