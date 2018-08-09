#include "light.h"

#include <math.h>

Light::Light(std::shared_ptr<Surface> surface)
    : m_surface(surface)
{}

Point3 Light::sample(RandomGenerator &random) const
{
    return m_surface->sample(random);
}
