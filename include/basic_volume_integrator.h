#pragma once

#include "bounce_controller.h"
#include "interaction.h"
#include "intersection.h"
#include "material.h"
#include "point.h"
#include "ray.h"
#include "random_generator.h"
#include "sample_integrator.h"
#include "scene.h"

class BasicVolumeIntegrator : public SampleIntegrator {
public:
    BasicVolumeIntegrator(BounceController bounceController)
        : m_bounceController(bounceController)
    {}

    Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const override;

private:
    Color L(
        const Point3 &point,
        const Vector3 &direction,
        const Scene &scene,
        RandomGenerator &random
    ) const;

    Interaction sampleInteraction2(
        const Point3 &sourcePoint,
        const Intersection &targetIntersection,
        const Ray &ray,
        const Scene &scene,
        RandomGenerator &random
    ) const;

    Interaction sampleInteraction(
        const Intersection &sourceIntersection,
        const Intersection &targetIntersection,
        const Ray &ray,
        RandomGenerator &random
    ) const;

    Color directSampleLights(
        const Interaction &interaction,
        const Scene &scene,
        RandomGenerator &random
    ) const;


    Color transmittance(
        const Intersection &sourceIntersection,
        const Intersection &targetIntersection
    ) const;

    BounceController m_bounceController;
};
