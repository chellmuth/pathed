#pragma once

#include "albedo.h"
#include "color.h"
#include "intersection.h"
#include "material.h"
#include "random_generator.h"
#include "vector.h"

#include <memory>

class Lambertian : public Material {
public:
    Lambertian(Color diffuse, Color emit);
    Lambertian(std::shared_ptr<Albedo> albedo, Color emit);

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
        os << "[Lambertian: diffuse=" << m_diffuse << " emit=" << m_emit << "]";
    }

private:
    Color m_diffuse;
    std::shared_ptr<Albedo> m_albedo;
};
