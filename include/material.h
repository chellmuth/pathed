#pragma once

#include "color.h"
#include "point.h"
#include "vector.h"

class Scene;

typedef struct {
    Vector3 normal;
    Point3 x;
    Vector3 wo;
    Vector3 wi;
} BSDFSample;

class Material {
public:
    Material(Color emit);

    virtual Color f(const Vector3 &wo, const Vector3 &wi, const Vector3 &normal) const = 0;
    Color emit() const;

protected:
    Color m_emit;
};
