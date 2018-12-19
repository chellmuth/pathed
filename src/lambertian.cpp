#include "lambertian.h"

#include <math.h>

Lambertian::Lambertian(Color diffuse, float specular, Color emit)
    : Material(emit), m_diffuse(diffuse), m_specular(specular)
{}

Color Lambertian::f(const Vector3 &wo, const Vector3 &wi) const
{
    return m_diffuse / M_PI;
}
