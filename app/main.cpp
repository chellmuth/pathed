#define  GL_SILENCE_DEPRECATION 1

#include <ctime>
#include <iostream>
#include <fstream>
#include <mutex>
#include <thread>
#include <vector>

#include <nanogui/opengl.h>
#include <nanogui/screen.h>
#include <nanogui/glcanvas.h>

#include "camera.h"
#include "canvas.h"
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

using namespace std;

static const int width = 768;
static const int height = 512;
static const int primarySamples = 50;
static const int bounceCount = 2;

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

void run(Image &image, Scene &scene, bool *quit)
{
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

        if (*quit) { return; }
    }
}

class PathApplication : public nanogui::Screen {
public:
    PathApplication(Image &image, int width, int height) : nanogui::Screen(Eigen::Vector2i(width, height), "Path Tracer", false) {
        using namespace nanogui;

        mCanvas = new Canvas(this, image, width, height);
        mCanvas->setSize({width, height});
        mCanvas->init();
        mCanvas->setBackgroundColor({100, 100, 100, 255});

        performLayout();
    }

    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {
        if (Screen::keyboardEvent(key, scancode, action, modifiers))
            return true;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            setVisible(false);
            return true;
        }

        return false;
    }

    virtual void draw(NVGcontext *ctx) {
        mCanvas->syncTextureBuffer();

        Screen::draw(ctx);
    }

private:
    Canvas *mCanvas;
};

int main() {
    printf("Hello, world!\n");

    Image image(width, height);

    ifstream jsonScene("mis.json");
    Scene scene = parseScene(jsonScene);

    bool quit = false;
    std::thread renderThread(run, std::ref(image), std::ref(scene), &quit);

    // image.debug();
    // image.write("test.bmp");

    try {
        nanogui::init();

        {
            nanogui::ref<PathApplication> app = new PathApplication(image, width, height);
            app->drawAll();
            app->setVisible(true);
            nanogui::mainloop();
        }

        nanogui::shutdown();
        quit = true;

    } catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        #if defined(_WIN32)
            MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
        #else
            std::cerr << error_msg << endl;
        #endif
        return -1;
    }

    renderThread.join();

    return 0;
}
