#include "light.h"

#include <math.h>

Light::Light(std::shared_ptr<Surface> surface)
    : m_surface(surface)
{}

SurfaceSample Light::sample(RandomGenerator &random) const
{
    return m_surface->sample(random);
}

Color Light::biradiance(const Point3 &lightPoint, const Point3 &surfacePoint) const
{
    Vector3 lightDirection = (surfacePoint - lightPoint).toVector();
    float distance = lightDirection.length();

    Vector3 lightNormal = Vector3(0.f, -1.f, 0.f);
    Vector3 wo = lightDirection.normalized();

    float cosineAttenuation = fmaxf(0.f, wo.dot(lightNormal));

    // g3d has 60 so I am off by 10x- where is that coming from?
    Color power = Color(1.f, 1.f, 1.f) * 600.f;
    return power * cosineAttenuation / (4.f * M_PI * distance * distance);
}
