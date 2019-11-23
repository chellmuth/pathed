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
        const Vector3 &wo,
        float *pdf
    ) const override;

    Vector3 sample(
        const Intersection &intersection,
        RandomGenerator &random,
        float *pdf
    ) const override;

private:
    Color m_diffuse;
    std::shared_ptr<Albedo> m_albedo;
};
