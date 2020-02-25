#pragma once

#include "color.h"
#include "intersection.h"
#include "random_generator.h"
#include "sample_integrator.h"
#include "scene.h"

class OptimalMISIntegrator : public SampleIntegrator {
public:
    OptimalMISIntegrator() {}

    Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const override;

private:
};
