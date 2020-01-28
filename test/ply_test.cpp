#include "ply_parser.h"

#include "catch.hpp"

#include <fstream>


TEST_CASE("read furry bunny", "[debug]") {
    std::ifstream objFile(
        "../assets/furry-bunny/bunny.ply",
        std::ios_base::binary
    );

    PLYParser parser(objFile);
    parser.parse();

    REQUIRE(true);
}
