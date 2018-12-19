#pragma once

#include "color.h"
#include "material.h"
#include "vector.h"

class Phong : public Material {
public:
    Phong(Color kd, Color ks, float n, Color emit);

    Color f(const Vector3 &wo, const Vector3 &wi, const Vector3 &normal) const;

private:
    Color m_kd, m_ks;
    float m_n;
};
