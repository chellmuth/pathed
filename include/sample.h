#pragma once

#include "point.h"

#include <vector>

struct ShadowTest {
    Point3 shadingPoint;
    Point3 lightPoint;
    bool occluded;
};

struct Sample {
    std::vector<Point3> eyePoints;
    std::vector<ShadowTest> shadowTests;

    Sample()
    : eyePoints(), shadowTests()
    {}
};
