#include "environment_light.h"

#include <math.h>

Color EnvironmentLight::emit() const
{
    return Color(0.f, 0.f, 1.f);
}

SurfaceSample EnvironmentLight::sample(const Intersection &intersection, RandomGenerator &random) const
{
    SurfaceSample fake = {
        .point = Point3(0.f, 0.f, 0.f),
        .normal = Vector3(0.f),
        .invPDF = 0.f
    };
    return fake;
}

SurfaceSample EnvironmentLight::sampleEmit(RandomGenerator &random) const
{
    SurfaceSample fake = {
        .point = Point3(0.f, 0.f, 0.f),
        .normal = Vector3(0.f),
        .invPDF = 0.f
    };
    return fake;
}

Color EnvironmentLight::biradiance(const SurfaceSample &lightSample, const Point3 &surfacePoint) const
{
    return emit();
}
