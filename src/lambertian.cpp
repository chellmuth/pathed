#include "lambertian.h"

#include <math.h>

Lambertian::Lambertian(Color diffuse, Color emit)
    : Material(emit), m_diffuse(diffuse)
{}

Color Lambertian::f(const Vector3 &wo, const Vector3 &wi, const Vector3 &normal) const
{
    return m_diffuse / M_PI;
}
