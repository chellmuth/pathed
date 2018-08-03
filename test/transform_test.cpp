#include "transform.h"

#include "vector.h"
#include "catch.hpp"

TEST_CASE("normalSpace maps (0, 1, 0) to normal", "[transform]") {
    Vector3 normal = Vector3(1, 2, 3).normalized();
    Transform transform = normalToWorldSpace(normal, Vector3(1, 0, 0));

    Vector3 expected(0.f, 1.f, 0.f);
    Vector3 transformedNormal = transform.apply(expected);

    REQUIRE(normal == transformedNormal);
}
