#include "fresnel.h"
#include "job.h"
#include "snell.h"
#include "tangent_frame.h"
#include "vector.h"

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

static json testFresnel(float etaTransmitted)
{
    json jsonData = json::array();

    const float etaIncident = 1.f;

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

    return jsonData;
}

static json testFresnel()
{
    json jsonData = json::array();

    jsonData.push_back({ { "legend", "1.1" }, { "data", testFresnel(1.1f) } });
    jsonData.push_back({ { "legend", "1.5" }, { "data", testFresnel(1.5f) } });
    jsonData.push_back({ { "legend", "2.0" }, { "data", testFresnel(2.f) } });

    return jsonData;
}

static json testSnell(float etaTransmitted)
{
    const float theta = M_PI / 6.f;
    const float etaIncident = 1.f;

    const float cosTheta = std::cos(theta);
    const Vector3 incident(-std::sqrt(std::max(0.f, 1.f - cosTheta * cosTheta)), cosTheta, 0.f);
    const float sinTheta = TangentFrame::sinTheta(incident);

    Vector3 transmitted(0.f);
    const bool doesRefract = Snell::refract(incident, &transmitted, etaIncident, etaTransmitted);
    std::cout << transmitted.toString() << std::endl;

    json jsonData;
    jsonData["incident"] = { incident.x(), incident.y(), incident.z() };
    jsonData["transmitted"] = { transmitted.x(), transmitted.y(), transmitted.z() };

    return jsonData;
}

static json testSnell()
{
    json jsonData = json::array();

    jsonData.push_back({ { "legend", "1.1" }, { "data", testSnell(1.1f) } });
    jsonData.push_back({ { "legend", "1.5" }, { "data", testSnell(1.5f) } });
    jsonData.push_back({ { "legend", "2.0" }, { "data", testSnell(2.f) } });

    return jsonData;
}

static void generateTestJson()
{
    json jsonData;
    jsonData["fresnel"] = testFresnel();
    jsonData["snell"] = testSnell();

    std::ofstream jsonFile("testbed.json");
    jsonFile << jsonData.dump(4) << std::endl;
    std::cout << "Wrote to testbed.json" << std::endl;
}

int main(int argc, char *argv[])
{
    std::cout << "Hello, world!" << std::endl;

    generateTestJson();

    return 0;
}
