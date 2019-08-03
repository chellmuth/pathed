#pragma once

#include "color.h"
#include "intersection.h"
#include "material.h"
#include "random_generator.h"
#include "texture.h"
#include "vector.h"

#include <memory>

class Mirror : public Material {
public:
    Mirror();

    Color f(
        const Intersection &intersection,
        const Vector3 &wo,
        float *pdf
    ) const override;

    Color sampleF(
        const Intersection &intersection,
        RandomGenerator &random,
        Vector3 *wi,
        float *pdf
    ) const override;
};
