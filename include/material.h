#pragma once

#include "color.h"
#include "vector.h"

class Scene;

class Material {
public:
    Material(Color diffuse, float specular, Color emit);

    Color f(const Vector3 &wo, const Vector3 &wi) const;
    Color emit() const;

private:
    Color m_diffuse;
    float m_specular;
    Color m_emit;
};
