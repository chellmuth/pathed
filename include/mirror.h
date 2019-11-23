#pragma once

#include "color.h"
#include "intersection.h"
#include "material.h"
#include "vector.h"

class Mirror : public Material {
public:
    Mirror();

    Color f(
        const Intersection &intersection,
        const Vector3 &wo,
        float *pdf
    ) const override;

    Vector3 sample(
        const Intersection &intersection,
        RandomGenerator &random,
        float *pdf
    ) const override;

private:
};
