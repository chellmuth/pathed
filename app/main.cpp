#define  GL_SILENCE_DEPRECATION 1

#include <ctime>
#include <iostream>
#include <fstream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <nanogui/opengl.h>
#include <nanogui/screen.h>
#include <nanogui/glcanvas.h>

#include "app_controller.h"
#include "bdpt.h"
#include "camera.h"
#include "canvas.h"
#include "color.h"
#include "depositer.h"
#include "image.h"
#include "integrator.h"
#include "intersection.h"
#include "monte_carlo.h"
#include "obj_parser.h"
#include "path_tracer.h"
#include "random_generator.h"
#include "rasterizer.h"
#include "ray.h"
#include "scene.h"
#include "scene_parser.h"
#include "transform.h"
#include "vector.h"

using namespace std;

static const int width = 400;
static const int height = 400;
static const int primarySamples = 99999;
static const int bounceCount = 4;

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

    Sample sample;
    sample.eyePoints.push_back(ray.origin());

    Color color = integrator.L(intersection, scene, random, bounceCount, sample);

    Color emit = intersection.material->emit();
    // Path = 1
    if (!emit.isBlack()) {
        // color += emit;
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
    Depositer integrator;

    {
        printf("Beginning pre-process...\n");
        std::clock_t begin = clock();
        integrator.preprocess(scene, random);
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

        sampleImage(
            radianceLookup,
            scene, integrator,
            random
        );

        std::clock_t end = clock();

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

        lock.unlock();
        double elapsedSeconds = double(end - begin) / CLOCKS_PER_SEC;
        printf("sample: %d/%d (%0.1fs elapsed)\n", i + 1, primarySamples, elapsedSeconds);

        if (*quit) { return; }
    }
}

class PathApplication : public nanogui::Screen {
public:
    PathApplication(Image &image, std::shared_ptr<AppController> controller, int width, int height)
        : nanogui::Screen(Eigen::Vector2i(width, height), "Path Tracer", false),
          mController(controller)
    {
        using namespace nanogui;

        mCanvas = new Canvas(this, controller, image, width, height);
        mCanvas->setSize({width, height});
        mCanvas->init();
        mCanvas->setBackgroundColor({100, 100, 100, 255});

        performLayout();
    }

    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {
        if (Screen::keyboardEvent(key, scancode, action, modifiers)) {
            return true;
        }

        if (key == GLFW_KEY_S && action == GLFW_PRESS) {
            mCanvas->save("output");
        }

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
    std::shared_ptr<AppController> mController;
};

class GLApplication : public nanogui::Screen {
public:
    GLApplication(Scene &scene, std::shared_ptr<AppController> controller, int width, int height)
        : nanogui::Screen(Eigen::Vector2i(width, height), "Rasterizer", false),
          mController(controller)
    {
        using namespace nanogui;

        mRasterizer = new Rasterizer(this, scene, width, height);
        mRasterizer->setSize({width, height});
        mRasterizer->init();
        mRasterizer->setBackgroundColor({100, 100, 100, 255});

        performLayout();
    }

    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {
        if (Screen::keyboardEvent(key, scancode, action, modifiers)) {
            return true;
        }

        if (key == GLFW_KEY_W) {
            mRasterizer->move(Direction::Forward);
        } else if (key == GLFW_KEY_S) {
            mRasterizer->move(Direction::Backward);
        } else if (key == GLFW_KEY_A) {
            mRasterizer->move(Direction::Left);
        } else if (key == GLFW_KEY_D) {
            mRasterizer->move(Direction::Right);
        }

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            setVisible(false);
            return true;
        }

        return false;
    }

    virtual void draw(NVGcontext *ctx) {
        if (mController->testAndClearUpdate()) {
            mRasterizer->reload();
        }

        mRasterizer->setState(mController->getSample());
        Screen::draw(ctx);
    }

private:
    Rasterizer *mRasterizer;
    std::shared_ptr<AppController> mController;
};

int main() {
    printf("Hello, world!\n");

    Image image(width, height);

    ifstream jsonScene("cornell.json");
    Scene scene = parseScene(jsonScene);

    bool quit = false;
    std::thread renderThread(run, std::ref(image), std::ref(scene), &quit);

    // image.debug();
    // image.write("test.bmp");

    auto controller = std::make_shared<AppController>(scene, width, height);

    try {
        nanogui::init();

        {
            nanogui::ref<PathApplication> app = new PathApplication(image, controller, width, height);
            app->drawAll();
            app->setVisible(true);

            nanogui::ref<GLApplication> debug = new GLApplication(scene, controller, width, height);
            debug->drawAll();
            debug->setVisible(true);

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
