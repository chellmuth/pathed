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
        const Vector3 &wi,
        float *pdf
    ) const override;

    BSDFSample sample(
        const Intersection &intersection,
        RandomGenerator &random
    ) const override;

private:
};
