#pragma once

#include "integrator.h"

class BDPT : public Integrator {
public:
    Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const override;
};
