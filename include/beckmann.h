#pragma once

#include "microfacet_distribution.h"
#include "random_generator.h"
#include "vector.h"

class Beckmann : public MicrofacetDistribution {
public:
    Vector3 sampleWh(float alpha, const Vector3 &wo, RandomGenerator &random) const override;
    float pdf(float alpha, const Vector3 &wh) const override;

    float D(const float alpha, const Vector3 &wh) const override;
    float G(float alphaX, float alphaY, const Vector3 &wo, const Vector3 &wi) const override;
    float lambda(float alphaX, float alphaY, const Vector3 &w) const override;
};
