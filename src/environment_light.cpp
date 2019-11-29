#include "environment_light.h"

#include "coordinate.h"
#include "monte_carlo.h"

#include <math.h>

Color EnvironmentLight::emit() const
{
    return Color(0.f, 0.f, 20.f);
}

SurfaceSample EnvironmentLight::sample(const Intersection &intersection, RandomGenerator &random) const
{
    Vector3 direction = UniformSampleSphere(random);

    SurfaceSample fake = {
        .point = intersection.point + (direction * 10000.f),
        .normal = direction * -1.f,
        .invPDF = UniformSampleSpherePDF(direction)
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
    Vector3 direction = (lightSample.point - surfacePoint).toVector();

    float phi, theta;
    cartesianToSpherical(direction, &phi, &theta);

    if (phi > M_PI) {
        return emit();
    }

    return Color(20.f, 0.f, 0.f);
}
