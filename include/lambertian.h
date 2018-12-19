#pragma once

#include "color.h"
#include "material.h"
#include "vector.h"

class Lambertian : public Material {
public:
    Lambertian(Color diffuse, float specular, Color emit);

    Color f(const Vector3 &wo, const Vector3 &wi) const;

private:
    Color m_diffuse;
    float m_specular;
};
