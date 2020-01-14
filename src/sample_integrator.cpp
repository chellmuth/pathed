#include "sample_integrator.h"

#include "camera.h"
#include "globals.h"
#include "job.h"

#include "omp.h"

void SampleIntegrator::samplePixel(
    int row, int col,
    int width, int height,
    std::vector<float> &radianceLookup,
    std::vector<Sample> &sampleLookup,
    const Scene &scene,
    RandomGenerator &random
) {
    Ray ray = scene.getCamera()->generateRay(row, col);

    Color color(0.f);

    Intersection intersection = scene.testIntersect(ray);
    if (intersection.hit) {
        Sample sample;
        sample.eyePoints.push_back(ray.origin());

        if (g_job->bounceController().checkCounts(0)) {
            Color emit = intersection.material->emit();
            if (!emit.isBlack() && !IntersectionHelper::checkBacksideIntersection(intersection)) {
                color += emit;
            }

            sample.contributions.push_back({color, 1.f});
        }

        color += L(intersection, scene, random, sample);

        sampleLookup[row * width + col] = sample;
    } else {
        color += scene.environmentL(ray.direction());
    }

    radianceLookup[3 * (row * width + col) + 0] += color.r();
    radianceLookup[3 * (row * width + col) + 1] += color.g();
    radianceLookup[3 * (row * width + col) + 2] += color.b();

    // Point3 point = intersection.point;
    // radianceLookup[3 * (row * width + col) + 0] += point.x();
    // radianceLookup[3 * (row * width + col) + 1] += point.y();
    // radianceLookup[3 * (row * width + col) + 2] += point.z();

    // Vector3 normal = intersection.shadingNormal;
    // radianceLookup[3 * (row * width + col) + 0] += 0.5f * (normal.x() + 1.f);
    // radianceLookup[3 * (row * width + col) + 1] += 0.5f * (normal.y() + 1.f);
    // radianceLookup[3 * (row * width + col) + 2] += 0.5f * (normal.z() + 1.f);
}

void SampleIntegrator::sampleImage(
    std::vector<float> &radianceLookup,
    std::vector<Sample> &sampleLookup,
    Scene &scene,
    RandomGenerator &random
) {
    const int width = g_job->width();
    const int height = g_job->height();

    // samplePixel(
    //     400 - 184 - 1, 97, // 0.0827, 0.0068, 0.0017
    //     width, height,
    //     radianceLookup,
    //     sampleLookup,
    //     scene,
    //     random
    // );
    // std::cout << "DONE!" << std::endl;

    // #pragma omp parallel for
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            samplePixel(
                row, col,
                width, height,
                radianceLookup,
                sampleLookup,
                scene,
                random
            );
            // std::cout << "row: " << row << " col: " << col << std::endl;
        }
    }
}
