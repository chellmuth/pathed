#include "lambertian.h"

#include "monte_carlo.h"
#include "transform.h"

#include <math.h>

Lambertian::Lambertian(Color diffuse, Color emit)
    : Material(emit), m_diffuse(diffuse), m_albedo(nullptr)
{}

Lambertian::Lambertian(std::shared_ptr<Albedo> albedo, Color emit)
    : Material(emit), m_diffuse(0.f), m_albedo(albedo)
{}

Color Lambertian::f(
    const Intersection &intersection,
    const Vector3 &wo,
    float *pdf
) const
{
    *pdf = UniformHemispherePdf(wo);

    if (m_albedo) {
        return m_albedo->lookup(intersection.uv) / M_PI;
    } else {
        return m_diffuse / M_PI;
    }
}

Vector3 Lambertian::sample(
    const Intersection &intersection,
    RandomGenerator &random,
    float *pdf
) const
{
    Transform tangentToWorld = normalToWorldSpace(
        intersection.normal,
        intersection.wi
    );

    Vector3 tangentSample = CosineSampleHemisphere(random);
    *pdf = CosineHemispherePdf(tangentSample);

    return tangentToWorld.apply(tangentSample);
}
