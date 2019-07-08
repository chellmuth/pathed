#pragma once

#include "point.h"
#include "uv.h"
#include "vector.h"

#include <limits>

class Material;

struct Intersection {
    bool hit;
    float t;
    Point3 point;
    Vector3 wi;
    Vector3 normal;
    UV uv;
    Material *material;
};

namespace IntersectionHelper {
    const Intersection miss = {
        false,
        std::numeric_limits<float>::max(),
        Point3(0.f, 0.f, 0.f),
        Vector3(0.f),
        Vector3(0.f),
        { 0.f, 0.f },
        nullptr
    };
}
