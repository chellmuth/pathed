#pragma once

#include <memory>

#include "color.h"
#include "material.h"
#include "point.h"
#include "random_generator.h"
#include "surface.h"

class Light {
public:
    Light(std::shared_ptr<Surface> surface);

    virtual std::shared_ptr<Material> getMaterial() const = 0;
    virtual SurfaceSample sample(RandomGenerator &random) const = 0;
    virtual Color biradiance(const SurfaceSample &lightSample, const Point3 &surfacePoint) const = 0;

protected:
    std::shared_ptr<Surface> m_surface;
};
