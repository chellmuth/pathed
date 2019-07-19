#pragma once

#include "sample_integrator.h"

class BDPT : public SampleIntegrator {
public:
    Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const override;
};
