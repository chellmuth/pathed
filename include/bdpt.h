#pragma once

#include "integrator.h"

class BDPT : public Integrator {
public:
    virtual Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        int bounceCount,
        Sample &sample
    ) const = 0;
};
