#include "plastic.h"

#include "monte_carlo.h"
#include "transform.h"

Plastic::Plastic(Color diffuse, float roughness)
    : Material(Color(0.f)),
      m_lambertian(diffuse, Color(0.f)),
      m_microfacet(roughness)
{}

Color Plastic::f(
    const Intersection &intersection,
    const Vector3 &wiWorld,
    float *pdf
) const
{
    if (intersection.woWorld.dot(intersection.shadingNormal) < 0.f) {
        *pdf = 0.f;
        return Color(0.f);
    }

    const Vector3 wi = intersection.worldToTangent.apply(wiWorld).normalized();
    *pdf = CosineHemispherePdf(wi);

    return m_lambertian.albedo(intersection) / M_PI;
}

BSDFSample Plastic::sample(
    const Intersection &intersection,
    RandomGenerator &random
) const
{
    Vector3 localSample = CosineSampleHemisphere(random);
    Vector3 worldSample = intersection.tangentToWorld.apply(localSample);

    BSDFSample sample = {
        .wiWorld = worldSample,
        .pdf = CosineHemispherePdf(localSample),
        .throughput = Material::f(intersection, worldSample),
        .material = this
    };

    return sample;
}
