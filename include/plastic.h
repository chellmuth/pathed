#pragma once

#include "albedo.h"
#include "color.h"
#include "intersection.h"
#include "lambertian.h"
#include "material.h"
#include "microfacet.h"
#include "random_generator.h"
#include "vector.h"

#include <memory>

class Plastic : public Material {
public:
    Plastic(Color diffuse, float roughness);
    Plastic(std::unique_ptr<Lambertian> lambertian, float roughness);

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
        os << "[Plastic lambertian=" << *m_lambertian << " microfacet=" << m_microfacet << "]";
    }

private:
    std::unique_ptr<Lambertian> m_lambertian;
    Microfacet m_microfacet;
};
