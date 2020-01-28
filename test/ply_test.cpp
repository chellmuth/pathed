#include "ply_parser.h"

#include "globals.h"

#include "catch.hpp"

#include <fstream>


TEST_CASE("ply tests", "[debug]") {
    g_rtcDevice = rtcNewDevice(NULL);
    g_rtcScene = rtcNewScene(g_rtcDevice);

    SECTION("read furry bunny") {
        std::ifstream objFile(
            "../assets/furry-bunny/bunny.ply",
            std::ios_base::binary
        );

        PLYParser parser(objFile);
        parser.parse();

        REQUIRE(true);
    }

    rtcReleaseScene(g_rtcScene);
    rtcReleaseDevice(g_rtcDevice);
}
