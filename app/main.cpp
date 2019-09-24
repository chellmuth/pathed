#define  GL_SILENCE_DEPRECATION 1

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

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>


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
    int sock = 0, valread;
    struct sockaddr_in serv_addr;

    float photonBundle[100];
    for (int i = 0; i < 100; i++) {
        photonBundle[i] = i / 99.f;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(65432);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    send(sock, photonBundle, sizeof(photonBundle), 0);
    printf("Hello message sent\n");

    float buffer[3] = {0.f, 0.f, 0.f};
    valread = recv(sock, buffer, sizeof(buffer), 0);
    float x = buffer[0];
    float y = buffer[1];
    float pdf = buffer[2];

    printf("%f %f %f\n", x, y, pdf);

    return 0;


    printf("Hello, world!\n");

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
