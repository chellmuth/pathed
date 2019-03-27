#pragma once

#include "color.h"
#include "material.h"
#include "vector.h"

class Lambertian : public Material {
public:
    Lambertian(Color diffuse, Color emit);

    Color f(const Vector3 &wo, const Vector3 &wi, const Vector3 &normal, float *pdf) const override;

private:
    Color m_diffuse;
};
