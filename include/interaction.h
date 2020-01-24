#pragma once

#include "point.h"
#include "medium.h"
#include "vector.h"

struct Interaction {
    bool isSurface;

    Point3 point;
    Vector3 woWorld;
    Vector3 wiWorld;
};
