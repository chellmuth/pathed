#include "material.h"

#include <math.h>
#include <stdio.h>

Material::Material(Color diffuse, float specular, Color emit)
    : m_diffuse(diffuse), m_specular(specular), m_emit(emit)
{}

Color Material::emit() const
{
    return m_emit;
}

Color Material::f(const Vector3 &wo, const Vector3 &wi) const
{
    return m_diffuse / M_PI;
}
