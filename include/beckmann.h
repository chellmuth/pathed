#pragma once

#include "microfacet_distribution.h"
#include "random_generator.h"
#include "vector.h"

class Beckmann : public MicrofacetDistribution {
public:
    Beckmann(float alpha);

    Vector3 sampleWh(const Vector3 &wo, RandomGenerator &random) const override;
    float pdf(const Vector3 &wi, const Vector3 &wh) const override;

    float D(const Vector3 &wi, const Vector3 &wh) const override;
    float G(const Vector3 &wo, const Vector3 &wi, const Vector3 &wh) const override;

private:
    float sampleAlpha(float absCosThetaI) const;
    float G1(const Vector3 &v) const;

    float m_alpha;
};
