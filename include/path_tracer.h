#pragma once

#include "bounce_controller.h"
#include "sample_integrator.h"

class PathTracer : public SampleIntegrator {
public:
    PathTracer(BounceController bounceController)
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
        const BSDFSample &bsdfSample,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const;

    BounceController m_bounceController;
};
