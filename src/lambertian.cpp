#include "lambertian.h"

#include "monte_carlo.h"

#include <math.h>

Lambertian::Lambertian(Color diffuse, Color emit)
    : Material(emit), m_diffuse(diffuse)
{}

Color Lambertian::f(const Vector3 &wo, const Vector3 &wi, const Vector3 &normal, float *pdf) const
{
    *pdf = UniformHemispherePdf();

    return m_diffuse / M_PI;
}
