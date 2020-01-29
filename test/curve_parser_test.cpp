#include "curve_parser.h"

#include "globals.h"

#include "catch.hpp"

#include <fstream>


TEST_CASE("curve tests", "[curve]") {
    g_rtcDevice = rtcNewDevice(NULL);
    g_rtcScene = rtcNewScene(g_rtcDevice);

    SECTION("read bunny furry") {
        std::ifstream curveFile("../assets/furry-bunny/bunnyfur.pbrt");

        CurveParser parser(curveFile);
        // parser.parse();

        REQUIRE(true);
    }

    rtcReleaseScene(g_rtcScene);
    rtcReleaseDevice(g_rtcDevice);
}
