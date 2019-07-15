#include "integrator.h"

#include "camera.h"
#include "globals.h"
#include "job.h"
#include "logger.h"
#include "ray.h"

#include <omp.h>

#include <ctime>
#include <fstream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <stdio.h>

static const int primarySamples = 99999;

void Integrator::run(Image &image, Scene &scene, std::function<void(RenderStatus)> callback, bool *quit)
{
    const int width = g_job->width();
    const int height = g_job->height();

    RandomGenerator random;

    {
        printf("Beginning pre-process...\n");
        std::clock_t begin = clock();
        preprocess(scene, random);
        std::clock_t end = clock();
        double elapsedSeconds = double(end - begin) / CLOCKS_PER_SEC;
        printf("Pre-process complete (%0.1fs elapsed)\n", elapsedSeconds);
    }

    std::vector<float> radianceLookup(3 * width * height);
    for (int i = 0; i < 3 * width * height; i++) {
        radianceLookup[i] = 0.f;
    }

    for (int i = 0; i < primarySamples; i++) {
        std::clock_t begin = clock();

        auto sampleLookup = std::make_shared<std::vector<Sample> >(width * height);
        sampleImage(
            radianceLookup,
            *sampleLookup,
            scene,
            random
        );

        std::clock_t end = clock();

        postwave(scene, random, i + 1);

        RenderStatus renderStatus;
        renderStatus.setSample(i + 1);
        renderStatus.setSampleLookup(sampleLookup);

        callback(renderStatus);

        std::mutex &lock = image.getLock();
        lock.lock();

        image.setSpp(i + 1);

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

        int maxJ = log2f(primarySamples);
        for (int j = 0; j <= maxJ; j++) {
            if (1 << j == i + 1) {
                image.save("auto");
            }
        }

        lock.unlock();
        double elapsedSeconds = double(end - begin) / CLOCKS_PER_SEC;

        std::ostringstream sampleStream;
        sampleStream << "sample: " << i + 1 << "/" << primarySamples
                     << std::fixed << std::setprecision(1)
                     << " (" << elapsedSeconds << "s elapsed)";

        Logger::line(sampleStream.str());

        if (*quit) { return; }
    }
}

void Integrator::samplePixel(
    int row, int col,
    int width, int height,
    std::vector<float> &radianceLookup,
    std::vector<Sample> &sampleLookup,
    const Scene &scene,
    RandomGenerator &random
) {
    Ray ray = scene.getCamera()->generateRay(
        row, col,
        width, height
    );

    Intersection intersection = scene.testIntersect(ray);
    if (!intersection.hit) { return; }

    Sample sample;
    sample.eyePoints.push_back(ray.origin());

    Color color = L(intersection, scene, random, sample);

    if (g_job->bounceController().checkCounts(0)) {
        Color emit = intersection.material->emit();
        if (!emit.isBlack()) {
            color += emit;
        }
    }

    sampleLookup[row * width + col] = sample;

    radianceLookup[3 * (row * width + col) + 0] += color.r();
    radianceLookup[3 * (row * width + col) + 1] += color.g();
    radianceLookup[3 * (row * width + col) + 2] += color.b();

    // Vector3 normal = intersection.normal;
    // radianceLookup[3 * (row * width + col) + 0] += 0.5f * (normal.x() + 1.f);
    // radianceLookup[3 * (row * width + col) + 1] += 0.5f * (normal.y() + 1.f);
    // radianceLookup[3 * (row * width + col) + 2] += 0.5f * (normal.z() + 1.f);
}

void Integrator::sampleImage(
    std::vector<float> &radianceLookup,
    std::vector<Sample> &sampleLookup,
    Scene &scene,
    RandomGenerator &random
) {
    const int width = g_job->width();
    const int height = g_job->height();

    #pragma omp parallel for
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
        }
    }
}
