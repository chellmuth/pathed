#pragma once

#include "color.h"
#include "intersection.h"
#include "material.h"
#include "random_generator.h"
#include "vector.h"

class OrenNayar : public Material {
public:
    OrenNayar();

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
    float m_A;
    float m_B;
};
