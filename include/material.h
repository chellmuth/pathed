#pragma once

#include "color.h"
#include "vector.h"

class Scene;

class Material {
public:
    Material(Color diffuse, Color emit);

    Color f(const Vector3 &wo, const Vector3 &wi) const;

private:
    Color m_diffuse;
    Color m_emit;
};
