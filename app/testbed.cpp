#include "job.h"

#include <embree3/rtcore.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

#include <iostream>

Job *g_job;
RTCDevice g_rtcDevice;
RTCScene g_rtcScene;

int main(int argc, char *argv[])
{
    std::cout << "Hello, world!" << std::endl;

    return 0;
}
