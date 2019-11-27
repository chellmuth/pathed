#pragma once

#include <memory>

#include "color.h"
#include "material.h"
#include "point.h"
#include "random_generator.h"
#include "surface.h"

class Light {
public:
    virtual Color emit() const = 0;
    virtual SurfaceSample sample(RandomGenerator &random) const = 0;
    virtual Color biradiance(const SurfaceSample &lightSample, const Point3 &surfacePoint) const = 0;
};
