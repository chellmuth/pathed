#include "phong.h"

#include <math.h>

#include "util.h"

Phong::Phong(Color kd, Color ks, float n, Color emit)
    : Material(emit), m_kd(kd), m_ks(ks), m_n(n)
{}

Color Phong::f(const Vector3 &wo, const Vector3 &wi, const Vector3 &normal) const
{
    Color diffuse = m_kd / M_PI;

    Vector3 reflected = wi.reflect(normal);
    float cosAlpha = fmaxf(0.f, wo.dot(reflected));
    Color specular = m_ks * (m_n + 2) / M_TWO_PI * powf(cosAlpha, m_n);

    return diffuse + specular;
}