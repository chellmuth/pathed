#pragma once

#include "color.h"
#include "intersection.h"
#include "vector.h"

class Scene;

class Material {
public:
    Material(Color emit);

    virtual Color f(
        const Intersection &intersection,
        const Vector3 &wo,
        float *pdf
    ) const = 0;

    Color f(const Intersection &intersection, const Vector3 &wo) {
        float pdf;
        return f(intersection, wo, &pdf);
    }

    Color emit() const;

protected:
    Color m_emit;
};
