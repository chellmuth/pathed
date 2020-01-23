#pragma once

#include "bounce_controller.h"
#include "interaction.h"
#include "intersection.h"
#include "material.h"
#include "medium.h"
#include "point.h"
#include "ray.h"
#include "random_generator.h"
#include "sample_integrator.h"
#include "scene.h"

#include <memory>

struct LoopState {
    int bounce;
    Intersection lastIntersection;
    Color modulation;
    std::shared_ptr<Medium> mediumPtr;
    Color result;
    BSDFSample bsdfSample;
    Interaction interaction;
};

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
    bool processBounce(
        LoopState &loopState,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const;

    bool processScatter(
        LoopState &state,
        const Scene &scene,
        RandomGenerator &random
    ) const;

    Color transmittance(
        const std::shared_ptr<Medium> &mediumPtr,
        const Intersection &sourceIntersection,
        const Intersection &targetIntersection
    ) const;

    IntegrationResult scatter(
        const std::shared_ptr<Medium> &mediumPtr,
        const Intersection &sourceIntersection,
        const Intersection &targetIntersection,
        const Scene &scene,
        RandomGenerator &random
    ) const;

    BounceController m_bounceController;
};
