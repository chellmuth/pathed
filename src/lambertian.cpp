#include "lambertian.h"

#include "monte_carlo.h"
#include "transform.h"

#include <cmath>

Lambertian::Lambertian(Color diffuse, Color emit)
    : Material(emit), m_diffuse(diffuse), m_albedo(nullptr)
{}

Lambertian::Lambertian(std::shared_ptr<Albedo> albedo, Color emit)
    : Material(emit), m_diffuse(0.f), m_albedo(albedo)
{}

Color Lambertian::f(
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
    *pdf = UniformHemispherePdf(wi);

    if (m_albedo) {
        return m_albedo->lookup(intersection) / M_PI;
    } else {
        return m_diffuse / M_PI;
    }
}

BSDFSample Lambertian::sample(
    const Intersection &intersection,
    RandomGenerator &random
) const
{
    Vector3 localSample = CosineSampleHemisphere(random);
    Vector3 worldSample = intersection.tangentToWorld.apply(localSample);

    BSDFSample sample = {
        .wiWorld = worldSample,
        .pdf = UniformHemispherePdf(localSample),
        .throughput = Material::f(intersection, worldSample),
        .material = this
    };

    return sample;
}
