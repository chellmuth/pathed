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
#include <iostream>
#include <mutex>

PDFIntegrator::PDFIntegrator()
{
    m_dataSource = std::make_shared<DataSource>();
}

Vector3 PDFIntegrator::sample(const Vector3 &normal, RandomGenerator &random, float *pdf) {
    Vector3 hemisphereSample = UniformSampleHemisphere(random);
    Transform hemisphereToWorld = normalToWorldSpace(normal);
    *pdf = INV_TWO_PI;
    return hemisphereToWorld.apply(hemisphereSample);
}

void PDFIntegrator::createPhotons(const Scene &scene, RandomGenerator &random)
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

            // float invPDF = 1.f / INV_TWO_PI;
            // throughput *= invPDF;
        }
    }
}

void PDFIntegrator::preprocess(const Scene &scene, RandomGenerator &random)
{
    createPhotons(scene, random);

    m_KDTree = std::make_unique<KDTree>(3, *m_dataSource, nanoflann::KDTreeSingleIndexAdaptorParams(10));
    m_KDTree->buildIndex();
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

    const int width = g_job->width();
    const int height = g_job->height();

    std::vector<float> radianceLookup(3 * width * height);
    for (int i = 0; i < 3 * width * height; i++) {
        radianceLookup[i] = 0.f;
    }

    Ray ray = scene.getCamera()->generateRay(200, 200);
    Intersection intersection = scene.testIntersect(ray);

    const int spp = 1024;
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

        int maxJ = log2f(spp);
        for (int j = 0; j <= maxJ; j++) {
            if (1 << j == i + 1) {
                image.save("auto");
            }
        }
    }

    *quit = true;
}

void PDFIntegrator::renderPDF(
    std::vector<float> &radianceLookup,
    const Scene &scene,
    const Intersection &intersection
) {
    const int phiSteps = g_job->width();
    const int thetaSteps = g_job->height();

    PathTracer pathTracer(g_job->bounceController());

    RandomGenerator random;
    int bounceCount = 2;

    Transform hemisphereToWorld = normalToWorldSpace(intersection.normal);

    #pragma omp parallel for
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

    const int debugSearchCount = g_job->debugSearchCount();
    auto resultIndices = std::make_shared<std::vector<size_t>>(debugSearchCount);
    std::vector<float> outDistanceSquared(debugSearchCount);

    nanoflann::KNNResultSet<float> resultSet(debugSearchCount);
    resultSet.init(resultIndices->data(), outDistanceSquared.data());

    Transform worldToNormal = worldSpaceToNormal(intersection.normal);

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
    photonPDF.save(worldToNormal);
}
