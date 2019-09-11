#include "pdf_integrator.h"

#include "camera.h"
#include "color.h"
#include "globals.h"
#include "job.h"
#include "path_tracer.h"
#include "random_generator.h"
#include "ray.h"
#include "sample.h"
#include "transform.h"
#include "util.h"
#include "vector.h"

#include <iostream>
#include <mutex>

void PDFIntegrator::run(
    Image &image,
    Scene &scene,
    std::function<void(RenderStatus)> callback,
    bool *quit
) {
    const int width = g_job->width();
    const int height = g_job->height();

    std::vector<float> radianceLookup(3 * width * height);
    for (int i = 0; i < 3 * width * height; i++) {
        radianceLookup[i] = 0.f;
    }

    Ray ray = scene.getCamera()->generateRay(500, 500);
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
}
