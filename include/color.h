#pragma once

#include "json.hpp"
using json = nlohmann::json;

class Color {
public:
    Color(json colorJson);
    Color(float r, float g, float b);

    float r() const { return m_r; }
    float g() const { return m_g; }
    float b() const { return m_b; }

private:
    float m_r, m_g, m_b;
};
