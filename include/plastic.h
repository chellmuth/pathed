#pragma once

#include "albedo.h"
#include "color.h"
#include "intersection.h"
#include "lambertian.h"
#include "material.h"
#include "microfacet.h"
#include "microfacet_distribution.h"
#include "random_generator.h"
#include "vector.h"

#include <memory>

class Plastic : public Material {
public:
    Plastic(Color diffuse, std::unique_ptr<MicrofacetDistribution> distributionPtr);
    Plastic(std::unique_ptr<Lambertian> lambertianPtr, std::unique_ptr<MicrofacetDistribution> distributionPtr);

    Color f(
        const Intersection &intersection,
        const Vector3 &wiWorld,
        float *pdf
    ) const override;

    BSDFSample sample(
        const Intersection &intersection,
        RandomGenerator &random
    ) const override;

    void writeStream(std::ostream &os) const override {
        os << "[Plastic lambertian=" << *m_lambertianPtr << " microfacet=" << m_microfacet << "]";
    }

private:
    std::unique_ptr<Lambertian> m_lambertianPtr;
    Microfacet m_microfacet;
};
