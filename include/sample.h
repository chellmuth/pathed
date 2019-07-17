#pragma once

#include "color.h"
#include "point.h"

#include <vector>

struct Contribution {
    Color radiance;
    float invPDF;
};

struct ShadowTest {
    Point3 shadingPoint;
    Point3 lightPoint;
    bool occluded;
};

struct Sample {
    std::vector<Point3> eyePoints;
    std::vector<ShadowTest> shadowTests;
    std::vector<Contribution> contributions;

    Sample()
    : eyePoints(), shadowTests(), contributions()
    {}
};
