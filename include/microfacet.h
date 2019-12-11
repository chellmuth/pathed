#pragma once

#include "color.h"
#include "intersection.h"
#include "material.h"
#include "random_generator.h"
#include "vector.h"

#include <memory>

class Microfacet : public Material {
public:
    Microfacet(float alphaX, float alphaY, Color emit) : Material(emit) {};

    Color f(
        const Intersection &intersection,
        const Vector3 &wi,
        float *pdf
    ) const override;

    BSDFSample sample(
        const Intersection &intersection,
        RandomGenerator &random
    ) const override;

private:
    float m_alphaX;
    float m_alphaY;
};
