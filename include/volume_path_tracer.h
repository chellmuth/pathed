#pragma once

#include "bounce_controller.h"
#include "intersection.h"
#include "material.h"
#include "point.h"
#include "ray.h"
#include "random_generator.h"
#include "sample_integrator.h"
#include "scene.h"

struct Interaction {
    bool isSurface;

    Point3 volumeInteraction;
};

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
    Interaction sampleInteraction(
        const Intersection &sourceIntersection,
        const Intersection &targetIntersection,
        const Ray &ray,
        RandomGenerator &random
    ) const;

    Color transmittance(
        const Intersection &sourceIntersection,
        const Intersection &targetIntersection
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
