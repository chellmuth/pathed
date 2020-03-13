#pragma once

#include "beckmann.h"
#include "color.h"
#include "intersection.h"
#include "material.h"
#include "microfacet_distribution.h"
#include "random_generator.h"
#include "vector.h"

#include <memory>

class Microfacet : public Material {
public:
    Microfacet(float alpha)
    : Material(0.f),
        m_distribution(std::make_unique<Beckmann>(alpha))
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
    std::unique_ptr<MicrofacetDistribution> m_distribution;
};
