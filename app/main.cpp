#define  GL_SILENCE_DEPRECATION 1

#include "app_controller.h"
#include "globals.h"
#include "image.h"
#include "integrator.h"
#include "job.h"
#include "render_status.h"
#include "scene.h"
#include "scene_parser.h"
#include "screen.h"

#include <embree3/rtcore.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <assert.h>
#include <memory>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace std;

Job *g_job;

void run(Image &image, Scene &scene, std::function<void(RenderStatus)> callback, bool *quit)
{
    std::unique_ptr<Integrator> integrator = g_job->integrator();
    integrator->run(image, scene, callback, quit);
}

int main() {
    printf("Hello, world!\n");

    RTCDevice rtcDevice = rtcNewDevice(NULL);
    if (rtcDevice == NULL) {
        std::cout << "Failed to create device" << std::endl;
        exit(1);
    }

    RTCScene rtcScene = rtcNewScene(rtcDevice);
    if (rtcScene == NULL) {
        std::cout << "Failed to create scene" << std::endl;
        exit(1);
    }

    int success = chdir("..");
    assert(success == 0);

    ifstream jsonJob("job.json");
    g_job = new Job(jsonJob);
    g_job->init();

    const int width = g_job->width();
    const int height = g_job->height();
    Image image(width, height);

    ifstream jsonScene(g_job->scene());
    Scene scene = parseScene(jsonScene);

    auto controller = std::make_shared<AppController>(scene, width, height);

    nanogui::init();
    nanogui::ref<PathedScreen> screen = new PathedScreen(
        image,
        scene,
        controller,
        width,
        height
    );

    std::function<void(RenderStatus)> callback([&screen](RenderStatus rs) {
        screen->updateRenderStatus(rs);
    });

    bool quit = false;
    std::thread renderThread(run, std::ref(image), std::ref(scene), callback, &quit);

    try {
       if (g_job->showUI()) {
            screen->drawAll();
            screen->setVisible(true);

            nanogui::mainloop();

            nanogui::shutdown();
            quit = true;
        }
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

    rtcReleaseScene(rtcScene);
    rtcReleaseDevice(rtcDevice);

    return 0;
}
