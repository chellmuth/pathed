#pragma once

#include "intersection.h"
#include "random_generator.h"
#include "scene.h"
#include "vector.h"

#include <vector>

typedef struct {
    Point3 origin;
    std::vector<Point3> bounceRays;
    std::vector<Point3> shadowRays;
} Sample;

class Integrator {
public:
    Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        int bounceCount,
        Sample &sample
    ) const;

private:
    Color direct(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const;
};
