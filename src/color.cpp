#include <assert.h>

#include "color.h"

Color::Color(float r, float g, float b)
    : m_r(r), m_g(g), m_b(b)
{}

bool Color::isBlack() const
{
    return m_r == 0.f && m_g == 0.f && m_b == 0.f;
}

Color Color::operator* (const Color c) const
{
    return Color(
        m_r * c.r(),
        m_g * c.g(),
        m_b * c.b()
    );
}

Color Color::operator* (const float t) const
{
    return Color(
        m_r * t,
        m_g * t,
        m_b * t
    );
}

Color Color::operator/ (const float t) const
{
    assert(t != 0.f);

    float invT = 1.f / t;

    return *this * invT;
}

std::ostream &operator<<(std::ostream &os, const Color &c)
{
    return os << "Color: " << c.r() << " " << c.g() << " " << c.b();
}
