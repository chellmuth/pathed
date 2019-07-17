#pragma once

#include "color.h"
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
    std::vector<Color> contributions;

    Sample()
    : eyePoints(), shadowTests(), contributions()
    {}
};
