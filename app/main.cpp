#include <iostream>
#include <fstream>
#include <mutex>
#include <thread>
#include <vector>

#include "camera.h"
#include "color.h"
#include "image.h"
#include "intersection.h"
#include "material.h"
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
static const int height = 300;
static const int primarySamples = 10;
static const int bounceCount = 1;

void sample(
    std::vector<float> &radianceLookup,
    Scene &scene, Camera &camera, RandomGenerator &random)
{
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            Ray ray = camera.generateRay(
                row, col,
                width, height
            );

            Intersection intersection = scene.testIntersect(ray);
            if (!intersection.hit) { continue; }
            // Vector3 normal = intersection.normal;
            // image.set(
            //     row,
            //     col,
            //     0.5f * (normal.x() + 1.f),
            //     0.5f * (normal.y() + 1.f),
            //     0.5f * (normal.z() + 1.f)
            // );

            Material material = *intersection.material;
            Color color = material.shade(intersection, scene, random);

            float bounceContribution = 1.f;
            Intersection bounceIntersection = intersection;

            for (int i = 0; i < bounceCount; i++) {
                Transform hemisphereToWorld = normalToWorldSpace(
                    intersection.normal,
                    ray.direction()
                );

                Vector3 hemisphereSample = UniformSampleHemisphere(random);
                Vector3 bounceDirection = hemisphereToWorld.apply(hemisphereSample);
                Ray bounceRay(
                    intersection.point,
                    bounceDirection
                );

                bounceIntersection = scene.testIntersect(bounceRay);
                if (!bounceIntersection.hit) { break; }

                material = *bounceIntersection.material;
                Color bounceColor = material.shade(bounceIntersection, scene, random);

                bounceContribution *= fmaxf(
                    0.f,
                    bounceRay.direction().dot(intersection.normal)
                );

                color = Color(
                    color.r() + bounceColor.r() * bounceContribution,
                    color.g() + bounceColor.g() * bounceContribution,
                    color.b() + bounceColor.b() * bounceContribution
                );

                intersection = bounceIntersection;
            }

            radianceLookup[3 * (row * width + col) + 0] += color.r();
            radianceLookup[3 * (row * width + col) + 1] += color.g();
            radianceLookup[3 * (row * width + col) + 2] += color.b();
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

    std::vector<float> radianceLookup(3 * width * height);
    for (int i = 0; i < 3 * width * height; i++) {
        radianceLookup[i] = 0.f;
    }

    for (int i = 0; i < primarySamples; i++) {
        sample(
            radianceLookup,
            scene, camera,
            random
        );

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
        printf("sample: (%d/%d)\n", i + 1, primarySamples);
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
