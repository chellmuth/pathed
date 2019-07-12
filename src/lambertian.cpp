#include "lambertian.h"

#include "monte_carlo.h"

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
