#pragma once

#include "color.h"
#include "material.h"
#include "vector.h"

class Phong : public Material {
public:
    Phong(Color diffuse, float specular, Color emit);

    Color f(const Vector3 &wo, const Vector3 &wi) const;

private:
    Color m_diffuse;
    float m_specular;
};
