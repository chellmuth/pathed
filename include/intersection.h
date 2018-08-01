#pragma once

#include "point.h"
#include "vector.h"

class Material;

struct Intersection {
    bool hit;
    float t;
    Point3 point;
    Vector3 normal;
    Material *material;
};
