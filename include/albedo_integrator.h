#pragma once

#include "color.h"
#include "intersection.h"
#include "random_generator.h"
#include "sample.h"
#include "sample_integrator.h"
#include "scene.h"

class AlbedoIntegrator : public SampleIntegrator {
public:
    Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        int pixelIndex,
        Sample &sample
    ) const override;
};
