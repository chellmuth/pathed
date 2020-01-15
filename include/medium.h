#pragma once

#include "color.h"
#include "point.h"
#include "random_generator.h"

class Scene;

struct TransmittanceQueryResult {
    bool isValid;
    float distance;
};

class Medium {
public:
    virtual Color transmittance(const Point3 &pointA, const Point3 &pointB) const = 0;
    virtual TransmittanceQueryResult findTransmittance(
        const Point3 &entryPointWorld,
        const Point3 &exitPointWorld,
        float targetTransmittance
    ) const = 0;

    virtual float sigmaT(const Point3 &worldPoint) const = 0;
    virtual float sigmaS(const Point3 &worldPoint) const = 0;

    virtual Color integrate(
        const Point3 &entryPointWorld,
        const Point3 &exitPointWorld,
        const Scene &scene,
        RandomGenerator &random
    ) const = 0;
};
