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
#include <sstream>
#include <stdio.h>

void Integrator::run(Image &image, Scene &scene, std::function<void(RenderStatus)> callback, bool *quit)
{
    const int width = g_job->width();
    const int height = g_job->height();

    const int primarySamples = g_job->spp();

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

        auto dataSource = std::make_shared<DataSource>(std::move(getDataSource()));
        renderStatus.setDataSource(dataSource);

        auto photons = std::make_shared<std::vector<DataSource::Point> >(getPhotons());
        renderStatus.setPhotons(photons);

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
                image.saveCheckpoint("auto");
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
