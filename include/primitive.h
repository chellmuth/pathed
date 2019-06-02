#pragma once

#include "aabb.h"
#include "intersection.h"
#include "ray.h"

class Primitive {
public:
    virtual Intersection testIntersect(const Ray &ray) const = 0;

    virtual Point3 centroid() const = 0;
    virtual void updateAABB(AABB *aabb) = 0;

    virtual void debug() const {}
};
