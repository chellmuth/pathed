#pragma once

#include "intersection.h"
#include "scene.h"
#include "random_generator.h"

class Integrator {
public:
    Color L(const Intersection &intersection, const Scene &scene, RandomGenerator &random) const;

private:
    Color Ld(const Intersection &intersection, const Scene &scene, RandomGenerator &random) const;
    Color indirect(const Intersection &intersection, const Scene &scene, RandomGenerator &random) const;
};
