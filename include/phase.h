#pragma once

#include "monte_carlo.h"
#include "random_generator.h"
#include "vector.h"

#include <cmath>

class Phase {
public:
    Phase() {}

    Vector3 sample(const Vector3 &woWorld, RandomGenerator &random) const {
        return UniformSampleSphere(random);
    }

    float f(const Vector3 &woWorld, const Vector3 &wiWorld) const {
        return UniformSampleSpherePDF(wiWorld);
    }
};
