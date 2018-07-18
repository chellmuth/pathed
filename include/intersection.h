#pragma once

#include "color.h"
#include "point.h"
#include "vector.h"

typedef struct {
    bool hit;
    float t;
    Point3 point;
    Vector3 normal;
    Color color;
} Intersection;
