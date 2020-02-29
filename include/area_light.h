#pragma once

#include "color.h"
#include "light.h"
#include "material.h"
#include "measure.h"
#include "point.h"
#include "random_generator.h"
#include "surface.h"

class AreaLight : public Light {
public:
    AreaLight(std::shared_ptr<Surface> surface);

    Color emit() const override;

    SurfaceSample sample(
        const Point3 &point,
        RandomGenerator &random
    ) const override;

    SurfaceSample sampleEmit(RandomGenerator &random) const override;
    float emitPDF(const Point3 &point, const Vector3 &direction, Measure measure) const override;

    Color biradiance(const SurfaceSample &lightSample, const Point3 &surfacePoint) const override;

private:
    std::shared_ptr<Surface> m_surface;
};
