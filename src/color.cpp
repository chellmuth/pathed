#include "color.h"

#include <assert.h>
#include <cmath>

Color::Color(float rgb)
    : m_r(rgb), m_g(rgb), m_b(rgb)
{}

Color::Color(float r, float g, float b)
    : m_r(r), m_g(g), m_b(b)
{}

bool Color::isBlack() const
{
    return m_r == 0.f && m_g == 0.f && m_b == 0.f;
}

Color Color::toLinear() const
{
    const float gamma = 2.2;

    const float linearR = m_r <= 0.04045f
        ?  m_r * 1.f / 12.92f
        : std::pow((m_r + 0.055f) * 1.f / 1.055f, gamma);

    const float linearG = m_g <= 0.04045f
        ?  m_g * 1.f / 12.92f
        : std::pow((m_g + 0.055f) * 1.f / 1.055f, gamma);

    const float linearB = m_b <= 0.04045f
        ?  m_b * 1.f / 12.92f
        : std::pow((m_b + 0.055f) * 1.f / 1.055f, gamma);

    return Color(linearR, linearG, linearB);
}

Color Color::toSRGB() const
{
    const float gamma = 2.2;

    const float linearR = m_r <= 0.0031308f
        ?  m_r * 12.92f
        : 1.055f * std::pow(m_r, (1.f / gamma)) - 0.055f;

    const float linearG = m_g <= 0.0031308f
        ?  m_g * 12.92f
        : 1.055f * std::pow(m_g, (1.f / gamma)) - 0.055f;

    const float linearB = m_b <= 0.0031308f
        ?  m_b * 12.92f
        : 1.055f * std::pow(m_b, (1.f / gamma)) - 0.055f;

    return Color(linearR, linearG, linearB);
}

Color Color::operator+ (const Color c) const
{
    return Color(
        m_r + c.r(),
        m_g + c.g(),
        m_b + c.b()
    );
}

Color Color::operator- (const Color c) const
{
    return Color(
        m_r - c.r(),
        m_g - c.g(),
        m_b - c.b()
    );
}

Color Color::operator- () const
{
    return Color(-m_r, -m_g, -m_b);
}

Color& Color::operator+= (const Color &c)
{
    m_r += c.r();
    m_g += c.g();
    m_b += c.b();

    return *this;
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

Color& Color::operator*= (const Color &c)
{
    m_r *= c.r();
    m_g *= c.g();
    m_b *= c.b();

    return *this;
}

Color Color::operator/ (const Color c) const
{
    assert(!c.isBlack());

    return Color(
        m_r / c.r(),
        m_g / c.g(),
        m_b / c.b()
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
