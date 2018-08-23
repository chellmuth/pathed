#pragma once

#include <memory>

#include "color.h"
#include "point.h"
#include "random_generator.h"
#include "surface.h"

class Light {
public:
    Light(std::shared_ptr<Surface> surface);

    SurfaceSample sample(RandomGenerator &random) const;
    Color biradiance(const Point3 &lightPoint, const Point3 &surfacePoint) const;

private:
    std::shared_ptr<Surface> m_surface;
};
