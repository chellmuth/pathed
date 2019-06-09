#pragma once

#include "intersection.h"
#include "random_generator.h"
#include "scene.h"
#include "point.h"

#include <vector>

typedef struct {
    Point3 origin;
    std::vector<Point3> bounceRays;
    std::vector<Point3> shadowRays;
} Sample;

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
