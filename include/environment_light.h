#pragma once

#include "color.h"
#include "intersection.h"
#include "light.h"
#include "material.h"
#include "point.h"
#include "random_generator.h"
#include "surface.h"

class EnvironmentLight : public Light {
public:
    EnvironmentLight() : Light() {};

    Color emit() const override;

    SurfaceSample sample(
        const Intersection &intersection,
        RandomGenerator &random
    ) const override;

    SurfaceSample sampleEmit(RandomGenerator &random) const override;

    Color biradiance(
        const SurfaceSample &lightSample,
        const Point3 &surfacePoint
    ) const override;
};
