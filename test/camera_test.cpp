#include "camera.h"
#include "point.h"
#include "vector.h"

#include "catch.hpp"

#include <cmath>
#include <iostream>

TEST_CASE("Camera hello", "[camera]") {
    Camera camera(
        Point3(0.f, 0.f, 0.f),
        Point3(0.f, 0.f, 10.f),
        Vector3(0.f, 1.f, 0.f),
        36.f / 180.f * M_PI,
        { 1, 1 },
        false
    );

    Pixel *pixel = nullptr;
    camera.calculatePixel(Point3(0.f, 0.f, 10.f), pixel);
    REQUIRE(pixel);

    REQUIRE(pixel->x == 0);
    REQUIRE(pixel->y == 0);
}

TEST_CASE("Camera miss", "[camera]") {
    Camera camera(
        Point3(0.f, 0.f, 0.f),
        Point3(0.f, 0.f, 10.f),
        Vector3(0.f, 1.f, 0.f),
        36.f / 180.f * M_PI,
        { 1, 1 },
        false
    );

    Pixel *pixel = nullptr;
    camera.calculatePixel(Point3(100000.f, 0.f, 10.f), pixel);
    REQUIRE(!pixel);
}

TEST_CASE("Camera hits a specific pixel easy", "[camera]") {
    Camera camera(
        Point3(0.f, 0.f, 0.f),
        Point3(0.f, 0.f, 10.f),
        Vector3(0.f, 1.f, 0.f),
        90.f / 180.f * M_PI,
        { 100, 100 },
        false
    );

    Pixel *pixel = nullptr;
    camera.calculatePixel(Point3(0.f, 0.f, 10.f), pixel);
    REQUIRE(pixel);

    REQUIRE(pixel->x == 50);
    REQUIRE(pixel->y == 49);
}

TEST_CASE("Camera hits a specific pixel tougher", "[camera]") {
    Camera camera(
        Point3(0.f, 0.f, 0.f),
        Point3(0.f, 0.f, 10.f),
        Vector3(0.f, 1.f, 0.f),
        90.f / 180.f * M_PI,
        { 100, 100 },
        false
    );

    Pixel *pixel = nullptr;
    camera.calculatePixel(Point3(1.f, 1.f, 2.f), pixel);
    REQUIRE(pixel);

    REQUIRE(pixel->x == 75);
    REQUIRE(pixel->y == 24);
}

TEST_CASE("Cornell light", "[camera]") {
    Camera camera(
        Point3(0.f, 1.f, 6.8f),
        Point3(0.f, 1.f, 0.f),
        Vector3(0.f, 1.f, 0.f),
        19.5f / 180.f * M_PI,
        { 100, 100 },
        false
    );

    auto lightPoint = Point3(0.174274f, 1.980000f, -0.149822f);
    Pixel *pixel;
    camera.calculatePixel(lightPoint, pixel);
    REQUIRE(pixel);

    REQUIRE(pixel->x == 57);
    REQUIRE(pixel->y == 92);
}
