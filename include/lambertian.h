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
    Lambertian(std::shared_ptr<Texture> texture, Color emit);

    Color f(
        const Intersection &intersection,
        const Vector3 &wo,
        float *pdf
    ) const override;

private:
    Color m_diffuse;
    std::shared_ptr<Texture> m_texture;
};
