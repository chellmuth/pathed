#include "color.h"
#include "point.h"
#include "uniform_grid.h"
#include "vector.h"

#include "catch.hpp"

#include <cmath>

TEST_CASE("uniform uniform grid", "[uniform]") {
    std::vector<float> gridData(10 * 10 * 10, 1.3f);

    UniformGrid grid(10, 10, 10, gridData);

    REQUIRE(grid.interpolate(3.4f, 2.f, 5.555f) == Approx(1.3f));
    REQUIRE(grid.interpolate(0.f, 0.f, 0.f) == Approx(1.3f));
}

TEST_CASE("grid bounds", "[uniform]") {
    std::vector<float> gridData(10 * 10 * 10, 1.3f);

    UniformGrid grid(10, 10, 10, gridData);

    REQUIRE(grid.interpolate(10.f, 10.f, 10.f) == Approx(0.f));
    REQUIRE(grid.interpolate(9.01f, 9.01f, 9.01f) == Approx(0.f));
    REQUIRE(grid.interpolate(9.f, 9.f, 9.f) == Approx(1.3f));
}
