#pragma once

#include "color.h"
#include "intersection.h"
#include "material.h"
#include "texture.h"
#include "vector.h"

#include <memory>

class Lambertian : public Material {
public:
    Lambertian(Color diffuse, Color emit);
    Lambertian(std::shared_ptr<Albedo> albedo, Color emit);

    Color f(
        const Intersection &intersection,
        const Vector3 &wi,
        float *pdf
    ) const override;

    BSDFSample sample(
        const Intersection &intersection,
        RandomGenerator &random
    ) const override;

private:
    Color m_diffuse;
    std::shared_ptr<Albedo> m_albedo;
};
