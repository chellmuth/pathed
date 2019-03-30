#include "material.h"

Material::Material(Color emit)
    : m_emit(emit)
{}

Color Material::emit() const
{
    return m_emit;
}
