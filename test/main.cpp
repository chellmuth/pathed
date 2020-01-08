#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "globals.h"
#include "job.h"

#include <embree3/rtcore.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

Job *g_job;
RTCDevice g_rtcDevice;
RTCScene g_rtcScene;

TEST_CASE( "Tests are running", "[pulse]" ) {
    REQUIRE( 1 == 1 );
}
