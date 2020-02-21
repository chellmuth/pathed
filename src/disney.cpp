#include "disney.h"

#include "monte_carlo.h"
#include "transform.h"

#include <cmath>

Disney::Disney(Color diffuse)
    : Material(0.f), m_diffuse(diffuse), m_albedo(nullptr)
{}

Disney::Disney(std::shared_ptr<Albedo> albedo)
    : Material(0.f), m_diffuse(0.f), m_albedo(albedo)
{}

Color Disney::f(
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

BSDFSample Disney::sample(
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

Color Disney::albedo(const Intersection &intersection) const
{
    if (m_albedo) {
        return m_albedo->lookup(intersection);
    } else {
        return m_diffuse;
    }
}
