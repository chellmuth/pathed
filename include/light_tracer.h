#pragma once

#include "bounce_controller.h"
#include "camera.h"
#include "integrator.h"
#include "intersection.h"
#include "random_generator.h"
#include "sample.h"
#include "scene.h"

#include <vector>

class LightTracer : public Integrator {
public:
    LightTracer(BounceController bounceController)
        : m_bounceController(bounceController)
    {}

private:
    void measure(
        std::vector<float> &radianceLookup,
        const Scene &scene,
        RandomGenerator &random
    ) const;

    void splat(
        const Color &radiance,
        const Intersection &intersection,
        const Scene &scene,
        std::vector<float> &radianceLookup
    ) const;

    void sampleImage(
        std::vector<float> &radianceLookup,
        std::vector<Sample> &sampleLookup,
        Scene &scene,
        RandomGenerator &random
    ) override;

    BounceController m_bounceController;
};
