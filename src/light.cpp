#include "light.h"

#include <math.h>

Light::Light(std::shared_ptr<Surface> surface)
    : m_surface(surface)
{}

std::shared_ptr<Material> Light::getMaterial() const
{
    return m_surface->getMaterial();
}

