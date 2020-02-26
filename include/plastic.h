#pragma once

#include "albedo.h"
#include "color.h"
#include "intersection.h"
#include "material.h"
#include "random_generator.h"
#include "vector.h"

#include <memory>

class Plastic : public Material {
public:
    Plastic(Color diffuse, float roughness);

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
        os << "[Plastic: diffuse=" << m_diffuse << "]";
    }

private:
    Color m_diffuse;
    float m_roughness;
};
