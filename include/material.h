#pragma once

#include "color.h"
#include "intersection.h"
#include "random_generator.h"
#include "vector.h"

class Scene;

struct BSDFSample {
    Vector3 wi;
    float pdf;
    Color throughput;
};

class Material {
public:
    Material(Color emit);

    virtual Color f(
        const Intersection &intersection,
        const Vector3 &wi,
        float *pdf
    ) const = 0;

    Color f(const Intersection &intersection, const Vector3 &wi) const {
        float pdf;
        return f(intersection, wi, &pdf);
    }

    virtual BSDFSample sample(
        const Intersection &intersection,
        RandomGenerator &random
    ) const = 0;

    Color emit() const;

protected:
    Color m_emit;
};
