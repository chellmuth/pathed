#pragma once

#include "intersection.h"
#include "random_generator.h"
#include "scene.h"
#include "point.h"

#include <vector>

struct Sample {
    Point3 origin;
    std::vector<Point3> bounceRays;
    std::vector<Point3> lightRays;
    std::vector<Point3> shadowRays;

    Sample(const Point3 &_origin) : origin(_origin) {}
};

class Integrator {
public:
    virtual Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        int bounceCount,
        Sample &sample
    ) const = 0;
};
