#pragma once

#include <memory>

#include "color.h"
#include "intersection.h"
#include "material.h"
#include "point.h"
#include "random_generator.h"
#include "surface.h"

class Light {
public:
    virtual Color emit() const = 0;

    virtual SurfaceSample sample(
        const Intersection &intersection,
        RandomGenerator &random
    ) const = 0;

    virtual SurfaceSample sampleEmit(RandomGenerator &random) const = 0;
    virtual float emitPDF(const Point3 &point, const Vector3 &direction) const = 0;

    // DEPRECATED
    virtual Color biradiance(
        const SurfaceSample &lightSample,
        const Point3 &surfacePoint
    ) const = 0;
};
