#include <iostream>

#include "image.h"

using namespace std;

int main() {
    printf("Hello, world!\n");

    int height = 300;
    int width = 400;

    Image image = Image(width, height);
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            image.set(
                row,
                col,
                row / (1.f * (height - 1)),
                col / (1.f * (width - 1)),
                1.f
            );
        }
    }

    // image.debug();
    image.write("test.bmp");

    return 0;
}
