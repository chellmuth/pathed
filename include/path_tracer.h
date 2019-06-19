#pragma once

#include "integrator.h"

class PathTracer : public Integrator {
public:
    Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        int bounceCount,
        Sample &sample
    ) const override;

    void debug(const Intersection &intersection, const Scene &scene) const override;

private:
    Color direct(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const;
};
