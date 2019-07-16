#pragma once

#include "point.h"

#include <vector>

struct Sample {
    std::vector<Point3> eyePoints;
    std::vector<Point3> shadowPoints;

    Sample()
    : eyePoints(), shadowPoints()
    {}
};
