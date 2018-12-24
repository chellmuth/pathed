#include <ctime>
#include <iostream>
#include <fstream>
#include <mutex>
#include <thread>
#include <vector>

#include "camera.h"
#include "color.h"
#include "image.h"
#include "intersection.h"
#include "integrator.h"
#include "monte_carlo.h"
#include "ray.h"
#include "scene.h"
#include "obj_parser.h"
#include "scene_parser.h"
#include "random_generator.h"
#include "transform.h"
#include "vector.h"
#include "window.h"

using namespace std;

static const int width = 768/2;
static const int height = 512/2;
static const int primarySamples = 100000;
static const int bounceCount = 0;

void samplePixel(
    int row, int col,
    std::vector<float> &radianceLookup,
    Scene &scene, Integrator &integrator,
    RandomGenerator &random)
{
    Ray ray = scene.getCamera()->generateRay(
        row, col,
        width, height
    );

    Intersection intersection = scene.testIntersect(ray);
    if (!intersection.hit) { return; }

    Color color = integrator.L(intersection, scene, random, bounceCount);

    Color emit = intersection.material->emit();
    if (!emit.isBlack()) {
        color = emit;
    }

    radianceLookup[3 * (row * width + col) + 0] += color.r();
    radianceLookup[3 * (row * width + col) + 1] += color.g();
    radianceLookup[3 * (row * width + col) + 2] += color.b();

    // Vector3 normal = intersection.normal;
    // radianceLookup[3 * (row * width + col) + 0] += 0.5f * (normal.x() + 1.f);
    // radianceLookup[3 * (row * width + col) + 1] += 0.5f * (normal.y() + 1.f);
    // radianceLookup[3 * (row * width + col) + 2] += 0.5f * (normal.z() + 1.f);
}

void sampleImage(
    std::vector<float> &radianceLookup,
    Scene &scene, Integrator &integrator,
    RandomGenerator &random)
{
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            samplePixel(row, col, radianceLookup, scene, integrator, random);
        }
    }
}

void run(Image &image)
{
    ifstream jsonScene("mis.json");
    Scene scene = parseScene(jsonScene);

    RandomGenerator random;
    Integrator integrator;

    std::vector<float> radianceLookup(3 * width * height);
    for (int i = 0; i < 3 * width * height; i++) {
        radianceLookup[i] = 0.f;
    }

    for (int i = 0; i < primarySamples; i++) {
        std::clock_t begin = clock();

        sampleImage(
            radianceLookup,
            scene, integrator,
            random
        );

        std::clock_t end = clock();

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
        double elapsedSeconds = double(end - begin) / CLOCKS_PER_SEC;
        printf("sample: %d/%d (%0.1fs elapsed)\n", i + 1, primarySamples, elapsedSeconds);
    }
}

int main() {
    printf("Hello, world!\n");

    Image image(width, height);

    std::thread renderThread(run, std::ref(image));

    // image.debug();
    // image.write("test.bmp");

    loop(image, width, height);

    renderThread.join();

    return 0;
}
