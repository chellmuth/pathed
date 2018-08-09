#pragma once

#include "point.h"
#include "random_generator.h"
#include "surface.h"

class Light {
public:
    Light(std::shared_ptr<Surface> surface);

    Point3 sample(RandomGenerator &random) const;

private:
    std::shared_ptr<Surface> m_surface;
};
