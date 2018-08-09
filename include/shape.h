#pragma once

#include "intersection.h"
#include "point.h"
#include "random_generator.h"

class Ray;

class Shape {
public:
    virtual Point3 sample(RandomGenerator &random) const = 0;
    virtual Intersection testIntersect(const Ray &ray) = 0;
};
