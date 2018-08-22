#pragma once

#include <ostream>

class Color {
public:
    Color(float r, float g, float b);

    float r() const { return m_r; }
    float g() const { return m_g; }
    float b() const { return m_b; }

    bool isBlack() const;

    Color operator* (const Color c) const;
    Color operator* (const float t) const;
    Color operator/ (const float t) const;

private:
    float m_r, m_g, m_b;
};

std::ostream &operator<<(std::ostream &os, const Color &c);

