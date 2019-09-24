#define  GL_SILENCE_DEPRECATION 1

#include "globals.h"
#include "image.h"
#include "integrator.h"
#include "job.h"
#include "ml_pdf.h"
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
RTCDevice g_rtcDevice;
RTCScene g_rtcScene;

void run(Image &image, Scene &scene, std::function<void(RenderStatus)> callback, bool *quit)
{
    std::unique_ptr<Integrator> integrator = g_job->integrator();
    integrator->run(image, scene, callback, quit);
}

int main(int argc, char *argv[]) {
    printf("Hello, world!\n");

    MLPDF pdf;
    pdf.connectToModel();
    pdf.go();

    g_rtcDevice = rtcNewDevice(NULL);
    if (g_rtcDevice == NULL) {
        std::cout << "Failed to create device" << std::endl;
        exit(1);
    }

    g_rtcScene = rtcNewScene(g_rtcDevice);
    if (g_rtcScene == NULL) {
        std::cout << "Failed to create scene" << std::endl;
        exit(1);
    }

    int success = chdir("..");
    assert(success == 0);

    if (argc > 1) {
        printf("Using: %s\n", argv[1]);

        ifstream jsonJob(argv[1]);
        g_job = new Job(jsonJob);
    } else {
        ifstream jsonJob("job.json");
        g_job = new Job(jsonJob);
    }

    g_job->init();

    const int width = g_job->width();
    const int height = g_job->height();
    Image image(width, height);

    ifstream jsonScene(g_job->scene());
    Scene scene = parseScene(jsonScene);

    nanogui::init();
    nanogui::ref<PathedScreen> screen = new PathedScreen(
        image,
        scene,
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

    rtcReleaseScene(g_rtcScene);
    rtcReleaseDevice(g_rtcDevice);

    return 0;
}
