#include <iostream>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "json.hpp"
using json = nlohmann::json;

#include "image.h"
#include "scene.h"

using namespace std;

int main() {
    if (!glfwInit()) {
        printf("GLFW init failed!\n");
        return 1;
    }
    
    const int height = 300;
    const int width = 400;

    GLFWwindow* window = glfwCreateWindow(width, height, "Path tracer", NULL, NULL);
    if (!window) {
        printf("GLFW window failed!\n");
        glfwTerminate();
        return 1;
    }

    while (!glfwWindowShouldClose(window)) {
        gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    printf("Hello, world!\n");

    Image image(width, height);

    ifstream scene_file("scene.json");
    Scene scene(json::parse(scene_file));

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
