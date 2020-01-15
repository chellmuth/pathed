#include "aabb.h"
#include "point.h"
#include "ray.h"
#include "vector.h"

#include "catch.hpp"

#include <cmath>

#define REQUIRE_POINT_APPROX(actual, expected) \
    REQUIRE(actual.x() == Approx(expected.x())); \
    REQUIRE(actual.y() == Approx(expected.y())); \
    REQUIRE(actual.z() == Approx(expected.z()));

TEST_CASE("simple hit", "[aabb]") {
    AABB aabb(
        0.f, 0.f, 0.f,
        1.f, 1.f, 1.f
    );

    Ray testRay(
        Point3(0.f, -0.5f, 0.f),
        Vector3(1.f, 1.f, 1.f).normalized()
    );

    AABBHit hit = aabb.intersect(testRay);

    REQUIRE(hit.hitCount == 2);
    REQUIRE_POINT_APPROX(hit.enterPoint, Point3(0.5, 0.f, 0.5f));
    REQUIRE_POINT_APPROX(hit.exitPoint, Point3(1.f, 0.5f, 1.f));
}

TEST_CASE("axis-aligned miss", "[aabb]") {
    AABB aabb(
        0.f, 0.f, 0.f,
        1.f, 1.f, 1.f
    );

    Ray testRay(
        Point3(-1.f, 0.f, 0.f),
        Vector3(0.f, 1.f, 0.f).normalized()
    );

    AABBHit hit = aabb.intersect(testRay);

    REQUIRE(hit.hitCount == 0);
}
