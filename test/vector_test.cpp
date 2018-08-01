#include "vector.h"

#include "catch.hpp"


TEST_CASE("Reflect", "[vector]") {
    Vector3 v(-0.5f, -0.5f, 0.f);
    Vector3 n(0.f, 1.f, 0.f);

    Vector3 reflected = v.reflect(n);

    Vector3 expected(-0.5f, 0.5f, 0.f);
    REQUIRE(expected == reflected);
}
