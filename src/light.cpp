#include "light.h"

Light::Light(std::shared_ptr<Surface> surface)
    : m_surface(surface)
{}

SurfaceSample Light::sample(RandomGenerator &random) const
{
    return m_surface->sample(random);
}
