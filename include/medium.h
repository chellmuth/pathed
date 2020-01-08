#pragma once

#include "color.h"
#include "point.h"

class Medium {
public:
    virtual Color transmittance(const Point3 &pointA, const Point3 &pointB) const = 0;
};
