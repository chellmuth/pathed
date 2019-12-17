#include "fresnel.h"
#include "job.h"

#include <embree3/rtcore.h>
#include "json.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

using json = nlohmann::json;

#include <cmath>
#include <iostream>

Job *g_job;
RTCDevice g_rtcDevice;
RTCScene g_rtcScene;

static void testFresnel()
{
    json jsonData = json::array();

    const float etaIncident = 1.f;
    const float etaTransmitted = 1.4;

    const int steps = 100;
    for (int i = 0; i < steps; i++) {
        const float theta = (i + 0.5f) / steps * (M_PI / 2.f);
        const float cosTheta = std::cos(theta);
        const float reflectance = Fresnel::dielectricReflectance(
            cosTheta,
            etaIncident,
            etaTransmitted
        );

        jsonData.push_back({{"x", theta}, {"y", reflectance }});
    }

    std::ofstream jsonFile("testbed-fresnel.json");
    jsonFile << jsonData.dump(4) << std::endl;
    std::cout << "Wrote to testbed-fresnel.json" << std::endl;

}

int main(int argc, char *argv[])
{
    std::cout << "Hello, world!" << std::endl;

    testFresnel();


    return 0;
}
