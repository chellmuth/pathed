#pragma once

#include "color.h"
#include "bounce_controller.h"
#include "intersection.h"
#include "random_generator.h"
#include "sample_integrator.h"
#include "sample.h"
#include "scene.h"

class MLIntegrator : public SampleIntegrator {
public:
    MLIntegrator(BounceController bounceController)
        : m_bounceController(bounceController)
    {}

    Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const override;

private:
    Color direct(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random
    ) const;

    BounceController m_bounceController;
};
