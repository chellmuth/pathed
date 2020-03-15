#include "beckmann.h"
#include "coordinate.h"
#include "fresnel.h"
#include "ggx.h"
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

static json testSnell(float etaIncident, float etaTransmitted)
{
    const float theta = 0.7f * (M_PI / 2.f);

    const float cosTheta = std::cos(theta);
    const Vector3 incident(-std::sqrt(std::max(0.f, 1.f - cosTheta * cosTheta)), cosTheta, 0.f);
    const float sinTheta = TangentFrame::sinTheta(incident);

    Vector3 transmitted(0.f);
    const bool doesRefract = Snell::refract(incident, &transmitted, etaIncident, etaTransmitted);
    std::cout << doesRefract << std::endl;
    std::cout << incident.toString() << " " << transmitted.toString() << std::endl;

    json jsonData;
    jsonData["incident"] = { incident.x(), incident.y(), incident.z() };
    jsonData["transmitted"] = { transmitted.x(), transmitted.y(), transmitted.z() };

    return jsonData;
}

static json testSnell()
{
    json jsonData = json::array();

    jsonData.push_back({ { "legend", "1.1" }, { "data", testSnell(1.1f, 1.f) } });
    jsonData.push_back({ { "legend", "1.5" }, { "data", testSnell(1.5f, 1.f) } });
    jsonData.push_back({ { "legend", "2.0" }, { "data", testSnell(2.f, 1.f) } });

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

static json generateDJson(const GGX &ggx, const Beckmann &beckmann)
{
    json ggxJson = json::array();
    json beckmannJson = json::array();

    const int sampleCount = 100;
    for (int i = 0; i < sampleCount; i++) {
        const float x = 1.f * i / sampleCount;
        const float theta = x * M_PI / 2.f;
        const Vector3 m = sphericalToCartesian(0.f, theta);

        const float ggxD = ggx.D(m);
        ggxJson.push_back({{ "x", x * 90 }, { "y", ggxD }});

        const float beckmannD = beckmann.D(m);
        beckmannJson.push_back({{ "x", x * 90 }, { "y", beckmannD }});
    }

    json jsonData = {
        { { "legend", "ggx" }, { "data", ggxJson } },
        { { "legend", "beckmann" }, { "data", beckmannJson } },
    };
    return jsonData;
}

static json generateG1Json(const GGX &ggx, const Beckmann &beckmann)
{
    json ggxJson = json::array();
    json beckmannJson = json::array();

    const int sampleCount = 100;
    for (int i = 0; i < sampleCount; i++) {
        const float x = 1.f * i / sampleCount;
        const float theta = x * M_PI / 2.f;
        const Vector3 v = sphericalToCartesian(0.f, theta);

        const float ggxG1 = ggx.G1(v);
        ggxJson.push_back({{ "x", x * 90 }, { "y", ggxG1 }});

        // const float beckmannD = beckmann.D(m);
        // beckmannJson.push_back({{ "x", x * 90 }, { "y", beckmannD }});
    }

    json jsonData = {
        { { "legend", "ggx" }, { "data", ggxJson } },
    };
    return jsonData;
}

static void generateMicrofacetChartData()
{
    const float alpha = 0.2f;

    GGX ggx(alpha);
    Beckmann beckmann(alpha);

    json jsonData;
    jsonData["D"] = generateDJson(ggx, beckmann);
    jsonData["G1"] = generateG1Json(ggx, beckmann);

    std::ofstream jsonFile("testbed-microfacet.json");
    jsonFile << jsonData.dump(4) << std::endl;
    std::cout << "Wrote to testbed-microfacet.json" << std::endl;
}

int main(int argc, char *argv[])
{
    std::cout << "Hello, world!" << std::endl;

    // generateTestJson();
    generateMicrofacetChartData();

    return 0;
}
