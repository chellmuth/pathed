#pragma once

#include "color.h"
#include "vector.h"

typedef struct {
    bool hit;
    float t;
    Vector3 normal;
    Color color;
} Intersection;
