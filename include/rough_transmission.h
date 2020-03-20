#pragma once

#include "color.h"
#include "intersection.h"
#include "material.h"
#include "microfacet_distribution.h"
#include "random_generator.h"
#include "vector.h"

#include <memory>

class RoughTransmission : public Material {
public:
    RoughTransmission(std::unique_ptr<MicrofacetDistribution> distributionPtr, float ior)
    : Material(0.f),
        m_distributionPtr(std::move(distributionPtr)),
        m_ior(ior)
    {}

    Color f(
        const Intersection &intersection,
        const Vector3 &wiWorld,
        float *pdf
    ) const override;

    BSDFSample sample(
        const Intersection &intersection,
        RandomGenerator &random
    ) const override;

private:
    float m_ior;
    std::unique_ptr<MicrofacetDistribution> m_distributionPtr;
};
