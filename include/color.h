#pragma once

class Color {
public:
    Color(float r, float g, float b);

    float r() const { return m_r; }
    float g() const { return m_g; }
    float b() const { return m_b; }

    Color operator* (const float t) const;

private:
    float m_r, m_g, m_b;
};
