#include "area_light.h"

#include <math.h>

SurfaceSample AreaLight::sample(RandomGenerator &random) const
{
    return m_surface->sample(random);
}

Color AreaLight::biradiance(const SurfaceSample &lightSample, const Point3 &surfacePoint) const
{
    Point3 lightPoint = lightSample.point;
    Vector3 lightDirection = (surfacePoint - lightPoint).toVector();
    float distance = lightDirection.length();

    Vector3 wo = lightDirection.normalized();

    float cosineAttenuation = fmaxf(0.f, wo.dot(lightSample.normal));

    Color radiance = m_surface->getRadiance();
    return radiance * cosineAttenuation / (distance * distance);
}
