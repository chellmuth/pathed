#pragma once

#include "random_generator.h"
#include "vector.h"

class MicrofacetDistribution {
public:
    virtual Vector3 sampleWh(float alpha, const Vector3 &wo, RandomGenerator &random) const = 0;
    virtual float pdf(float alpha, const Vector3 &wh) const = 0;

    virtual float D(const float alpha, const Vector3 &wh) const = 0;
    virtual float G(float alphaX, float alphaY, const Vector3 &wo, const Vector3 &wi) const = 0;
    virtual float lambda(float alphaX, float alphaY, const Vector3 &w) const = 0;
};
