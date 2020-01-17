#pragma once

#include <ostream>

class Color {
public:
    Color(float rgb);
    Color(float r, float g, float b);

    float r() const { return m_r; }
    float g() const { return m_g; }
    float b() const { return m_b; }

    bool isBlack() const;

    float luminance() const {
        return (
            0.2126f * m_r +
            0.7152f * m_g +
            0.0722f * m_b
        );
    }

    float average() const {
        return (m_r + m_g + m_b) / 3.f;
    }

    Color& operator+= (const Color &c);
    Color operator+ (const Color c) const;
    Color operator- (const Color c) const;
    Color operator- () const;
    Color operator* (const Color c) const;
    Color operator* (const float t) const;
    Color& operator*= (const Color &c);
    Color operator/ (const float t) const;

private:
    float m_r, m_g, m_b;
};

std::ostream &operator<<(std::ostream &os, const Color &c);
