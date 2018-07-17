#include <iostream>
#include <fstream>

#include "json.hpp"
using json = nlohmann::json;

#include "color.h"
#include "image.h"
#include "intersection.h"
#include "ray.h"
#include "scene.h"
#include "scene_parser.h"
#include "vector.h"
#include "window.h"

using namespace std;

int main() {
    printf("Hello, world!\n");

    const int width = 400;
    const int height = 300;

    Image image(width, height);

    ifstream scene_file("scene.json");
    Scene scene = parseScene(json::parse(scene_file));

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            Ray ray(
                Point3(col, row, 0.f),
                Vector3(0.f, 0.f, 1.f)
            );

            Intersection intersection = scene.testIntersect(ray);
            if (intersection.hit) {
                // Vector3 normal = intersection.normal;
                // image.set(
                //     row,
                //     col,
                //     0.5f * (normal.x() + 1.f),
                //     0.5f * (normal.y() + 1.f),
                //     0.5f * (normal.z() + 1.f)
                // );

                Color color = intersection.color;
                image.set(
                    row,
                    col,
                    color.r(),
                    color.b(),
                    color.g()
                );
            } else {
                image.set(row, col, 0.f, 0.f, 0.f);
            }
        }
    }

    // image.debug();
    // image.write("test.bmp");

    return loop(image.data());
}
