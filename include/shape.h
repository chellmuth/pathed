#pragma once

#include "intersection.h"
#include "point.h"
#include "random_generator.h"
#include "vector.h"

class Ray;

typedef struct {
    Point3 point;
    Vector3 normal;
} SurfaceSample;

class Shape {
public:
    virtual SurfaceSample sample(RandomGenerator &random) const = 0;
    virtual Intersection testIntersect(const Ray &ray) = 0;
};
