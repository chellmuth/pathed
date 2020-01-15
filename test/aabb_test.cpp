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

TEST_CASE("axis-aligned hit", "[aabb]") {
    AABB aabb(
        0.f, 0.f, 0.f,
        1.f, 1.f, 1.f
    );

    Ray testRay(
        Point3(0.5f, -0.5f, 0.5f),
        Vector3(0.f, 1.f, 0.f).normalized()
    );

    AABBHit hit = aabb.intersect(testRay);

    REQUIRE(hit.hitCount == 2);
    REQUIRE_POINT_APPROX(hit.enterPoint, Point3(0.5f, 0.0f, 0.5f));
    REQUIRE_POINT_APPROX(hit.exitPoint, Point3(0.5f, 1.f, 0.5f));
}

TEST_CASE("face-aligned hit", "[aabb][!shouldfail]") {
    AABB aabb(
        0.f, 0.f, 0.f,
        1.f, 1.f, 1.f
    );

    Ray testRay(
        Point3(0.f, -0.5f, 0.f),
        Vector3(0.f, 1.f, 0.f).normalized()
    );

    AABBHit hit = aabb.intersect(testRay);

    REQUIRE(hit.hitCount == 2);
    REQUIRE_POINT_APPROX(hit.enterPoint, Point3(0.f, 0.f, 0.f));
    REQUIRE_POINT_APPROX(hit.exitPoint, Point3(0.f, 1.f, 0.f));
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

TEST_CASE("conventional miss", "[aabb]") {
    AABB aabb(
        0.f, 0.f, 0.f,
        1.f, 1.f, 1.f
    );

    Ray testRay(
        Point3(-10.f, -10.f, -10.f),
        Vector3(1.f, 2.f, 3.f).normalized()
    );

    AABBHit hit = aabb.intersect(testRay);

    REQUIRE(hit.hitCount == 0);
}

TEST_CASE("ray starts inside", "[aabb]") {
    AABB aabb(
        0.f, 0.f, 0.f,
        1.f, 1.f, 1.f
    );

    Ray testRay(
        Point3(0.5f, 0.4f, 0.3f),
        Vector3(1.f, 1.f, 1.f).normalized()
    );

    AABBHit hit = aabb.intersect(testRay);

    REQUIRE(hit.hitCount == 1);
    REQUIRE_POINT_APPROX(hit.enterPoint, testRay.origin());
    REQUIRE_POINT_APPROX(hit.exitPoint, Point3(1.f, 0.9f, 0.8f));
}

TEST_CASE("ray moving backwards", "[aabb]") {
    AABB aabb(
        0.f, 0.f, 0.f,
        1.f, 1.f, 1.f
    );

    Ray testRay(
        Point3(1.5f, 1.4f, 1.3f),
        Vector3(-1.f, -1.f, -1.f).normalized()
    );

    AABBHit hit = aabb.intersect(testRay);

    REQUIRE(hit.hitCount == 2);
    REQUIRE_POINT_APPROX(hit.enterPoint, Point3(1.f, 0.9f, 0.8f));
    REQUIRE_POINT_APPROX(hit.exitPoint, Point3(0.2f, 0.1f, 0.f));
}

TEST_CASE("intersect corner", "[aabb]") {
    AABB aabb(
        0.f, 0.f, 0.f,
        1.f, 1.f, 1.f
    );

    Ray testRay(
        Point3(1.f, 1.f, -1.f),
        Vector3(-1.f, -1.f, 1.f).normalized()
    );

    AABBHit hit = aabb.intersect(testRay);

    REQUIRE(hit.hitCount == 0);
}

TEST_CASE("ray starts on bounds", "[aabb]") {
    AABB aabb(
        0.f, 0.f, 0.f,
        1.f, 1.f, 1.f
    );

    Ray testRay(
        Point3(0.5f, 0.f, 0.4f),
        Vector3(1.f, 1.f, 1.f).normalized()
    );

    AABBHit hit = aabb.intersect(testRay);

    REQUIRE(hit.hitCount == 2);
    REQUIRE_POINT_APPROX(hit.enterPoint, Point3(0.5f, 0.f, 0.4f));
    REQUIRE_POINT_APPROX(hit.exitPoint, Point3(1.f, 0.5f, 0.9f));
}

TEST_CASE("custom bounds", "[aabb]") {
    AABB aabb(
        -1.f, 0.f, 6.f,
        1.f, 4.f, 100.f
    );

    Ray testRay(
        Point3(-3.f, 1.f, 5.f),
        Vector3(1.f, 1.f, 1.f).normalized()
    );

    AABBHit hit = aabb.intersect(testRay);

    REQUIRE(hit.hitCount == 2);
    REQUIRE_POINT_APPROX(hit.enterPoint, Point3(-1.f, 3.f, 7.f));
    REQUIRE_POINT_APPROX(hit.exitPoint, Point3(0.f, 4.f, 8.f));
}

TEST_CASE("intersect segment both inside", "[aabb]") {
    AABB aabb(
        0.f, 0.f, 0.f,
        1.f, 1.f, 1.f
    );

    Point3 enterPoint = Point3(0.3f, 0.3f, 0.3f);
    Point3 exitPoint = Point3(0.4f, 0.5f, 0.6f);

    AABBHit hit = aabb.intersect(enterPoint, exitPoint);

    REQUIRE(hit.hitCount == 2);
    REQUIRE_POINT_APPROX(hit.enterPoint, Point3(0.3f, 0.3f, 0.3f));
    REQUIRE_POINT_APPROX(hit.exitPoint, Point3(0.4f, 0.5f, 0.6f));
}
