#pragma once

#include "color.h"
#include "vector.h"

class Scene;

class Material {
public:
    Material(Color emit);

    virtual Color f(const Vector3 &wo, const Vector3 &wi, const Vector3 &normal, float *pdf) const = 0;
    Color f(const Vector3 &wo, const Vector3 &wi, const Vector3 &normal) {
        float pdf;
        return f(wo, wi, normal, &pdf);
    }

    Color emit() const;

protected:
    Color m_emit;
};
