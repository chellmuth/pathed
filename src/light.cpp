#include "light.h"

#include <math.h>

Light::Light(std::shared_ptr<Surface> surface)
    : m_surface(surface)
{}
