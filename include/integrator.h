#pragma once

#include "intersection.h"
#include "random_generator.h"
#include "scene.h"
#include "point.h"

#include <vector>

struct Sample {
    std::vector<Point3> eyePoints;
    std::vector<Point3> lightPoints;
    std::vector<Point3> shadowPoints;
    bool connected;

    Sample()
    : eyePoints(), lightPoints(), shadowPoints(), connected(false)
    {}
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
