#pragma once

#include "color.h"
#include "vector.h"

class Scene;

class Material {
public:
    Material(Color emit);

    virtual Color f(const Vector3 &wo, const Vector3 &wi) const = 0;
    Color emit() const;

protected:
    Color m_emit;
};
