#pragma once

#include "bounce_controller.h"
#include "material.h"
#include "sample_integrator.h"

class VolumePathTracer : public SampleIntegrator {
public:
    VolumePathTracer(BounceController bounceController)
        : m_bounceController(bounceController)
    {}

    Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const override;

private:
    Color transmittance(
        const Material *materialOut,
        const Material *materialIn,
        const Point3 &source,
        const Point3 &target
    ) const;

    Color direct(
        const Intersection &intersection,
        const BSDFSample &bsdfSample,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const;

    Color directSampleLights(
        const Intersection &intersection,
        const BSDFSample &bsdfSample,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const;

    Color directSampleBSDF(
        const Intersection &intersection,
        const BSDFSample &bsdfSample,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const;

    BounceController m_bounceController;
};
