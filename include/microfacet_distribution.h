#pragma once

#include "random_generator.h"
#include "vector.h"

class MicrofacetDistribution {
public:
    virtual Vector3 sampleWh(const Vector3 &wo, RandomGenerator &random) const = 0;
    virtual float pdf(const Vector3 &wi, const Vector3 &wh) const = 0;

    virtual float D(const Vector3 &wi, const Vector3 &wh) const = 0;
    virtual float G(const Vector3 &wo, const Vector3 &wi, const Vector3 &wh) const = 0;
};
