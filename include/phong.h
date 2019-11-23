#pragma once

#include "color.h"
#include "intersection.h"
#include "material.h"
#include "random_generator.h"

class Phong : public Material {
public:
    Phong(Color kd, Color ks, float n, Color emit);

    Color f(
        const Intersection &intersection,
        const Vector3 &wo,
        float *pdf
    ) const override;

    BSDFSample sample(
        const Intersection &intersection,
        RandomGenerator &random
    ) const override;

private:
    Color m_kd, m_ks;
    float m_n;
};
