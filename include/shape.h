#pragma once

#include "intersection.h"

class Ray;

class Shape {
public:
    virtual Intersection testIntersect(const Ray &ray) = 0;
};
