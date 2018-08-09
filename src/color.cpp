#include "color.h"

Color::Color(float r, float g, float b)
    : m_r(r), m_g(g), m_b(b)
{}

bool Color::isBlack() const
{
    return m_r == 0.f && m_g == 0.f && m_b == 0.f;
}

Color Color::operator* (const float t) const
{
    return Color(
        m_r * t,
        m_g * t,
        m_b * t
    );
}
