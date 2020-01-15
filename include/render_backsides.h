#pragma once

#include "intersection.h"
#include "scene.h"
#include "sample.h"
#include "sample_integrator.h"
#include "random_generator.h"

class RenderBacksides : public SampleIntegrator {
public:
    RenderBacksides() {}

    Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const override;

private:
};
