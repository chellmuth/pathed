#include "color.h"

Color::Color(json pointJson)
{
    m_r = pointJson[0];
    m_g = pointJson[1];
    m_b = pointJson[2];
}

Color::Color(float r, float g, float b)
    : m_r(r), m_g(g), m_b(b)
{}
