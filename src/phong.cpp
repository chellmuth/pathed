#include "phong.h"

#include "monte_carlo.h"
#include "util.h"

#include <math.h>

Phong::Phong(Color kd, Color ks, float n, Color emit)
    : Material(emit), m_kd(kd), m_ks(ks), m_n(n)
{}

Color Phong::f(const Intersection &intersection, const Vector3 &wo, float *pdf) const
{
    const Vector3 &wi = intersection.wi;
    const Vector3 &normal = intersection.normal;

    Color diffuse = m_kd / M_PI;

    Vector3 reflected = wi.reflect(normal);
    float cosAlpha = fmaxf(0.f, wo.dot(reflected));
    Color specular = m_ks * (m_n + 2) / M_TWO_PI * powf(cosAlpha, m_n);

    *pdf = UniformHemispherePdf(wo);

    return diffuse + specular;
}

Color Phong::sampleF(
    const Intersection &intersection,
    RandomGenerator &random,
    Vector3 *wi,
    float *pdf
) const
{
    throw "Unimplemented!";
}
