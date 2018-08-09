#pragma once

#include "surface.h"

class Light {
public:
    Light(std::shared_ptr<Surface> surface);

private:
    std::shared_ptr<Surface> m_surface;
};
