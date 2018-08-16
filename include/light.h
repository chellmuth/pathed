#pragma once

#include <memory>

#include "point.h"
#include "random_generator.h"
#include "surface.h"

class Light {
public:
    Light(std::shared_ptr<Surface> surface);

    SurfaceSample sample(RandomGenerator &random) const;

private:
    std::shared_ptr<Surface> m_surface;
};
