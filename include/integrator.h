#pragma once

#include "intersection.h"
#include "random_generator.h"
#include "scene.h"
#include "vector.h"

#include <vector>

class Integrator {
public:
    Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        int bounceCount,
        std::vector<Vector3> &intersectionList
    ) const;

private:
    Color direct(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        std::vector<Vector3> &intersectionList
    ) const;
};
