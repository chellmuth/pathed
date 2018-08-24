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
#include "random_generator.h"
#include "transform.h"
#include "vector.h"
#include "window.h"

using namespace std;

static const int width = 400;
static const int height = 400;
static const int primarySamples = 50;
static const int bounceCount = 2;

void samplePixel(
    int row, int col,
    std::vector<float> &radianceLookup,
    Scene &scene, Integrator &integrator,
    Camera &camera, RandomGenerator &random)
{
    Ray ray = camera.generateRay(
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
}

void sampleImage(
    std::vector<float> &radianceLookup,
    Scene &scene, Integrator &integrator,
    Camera &camera, RandomGenerator &random)
{
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            samplePixel(row, col, radianceLookup, scene, integrator, camera, random);
        }
    }
}

void run(Image &image)
{
    ifstream sceneFile("CornellBox-Original.obj");
    ObjParser objParser(sceneFile, Handedness::Left);
    Scene scene = objParser.parseScene();

    Transform cameraToWorld = lookAt(
        Point3(0.f, 1.f, 3.6f),
        Point3(0.f, 1.f, 0.f),
        Vector3(0.f, 1.f, 0.f)
    );
    Camera camera(cameraToWorld, 45 / 180.f * M_PI);
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
            camera, random
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
