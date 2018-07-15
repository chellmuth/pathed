#include <iostream>

#include "image.h"
#include "scene.h"

using namespace std;

int main() {
    printf("Hello, world!\n");

    int height = 300;
    int width = 400;

    Image image(width, height);

    Scene scene;
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            Ray ray(
                Point3(col, row, 0.f),
                Vector3(0.f, 0.f, 1.f)
            );

            bool intersects = scene.testIntersect(ray);
            if (intersects) {
                image.set(row, col, 1.f, 1.f, 1.f);
            } else {
                image.set(row, col, 0.f, 0.f, 0.f);
            }
        }
    }

    // image.debug();
    image.write("test.bmp");

    return 0;
}
