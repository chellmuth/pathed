#pragma once

#include "intersection.h"
#include "scene.h"
#include "random_generator.h"

class Integrator {
public:
    Color L(const Intersection &intersection, const Scene &scene, RandomGenerator &random, int bounceCount) const;

private:
    Color direct(const Intersection &intersection, const Scene &scene, RandomGenerator &random) const;
};