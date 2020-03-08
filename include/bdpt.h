#pragma once

#include "sample_integrator.h"

class BDPT : public SampleIntegrator {
public:
    Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        int pixelIndex,
        Sample &sample
    ) const override;
};
