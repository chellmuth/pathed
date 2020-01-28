#pragma once

#include "intersection.h"
#include "ray.h"

class Primitive {
public:
    virtual Intersection testIntersect(const Ray &ray) const = 0;

    virtual void debug() const {}
};
