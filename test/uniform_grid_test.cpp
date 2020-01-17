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
}
