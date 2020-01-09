#pragma once

#include "point.h"
#include "vector.h"

struct Interaction {
    bool isSurface;

    Point3 point;
    Vector3 direction;
    float pdf;

    float sigmaS;
    float sigmaT;
};
