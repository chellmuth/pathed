#pragma once

#include "point.h"

class Vector3 {
public:
    Vector3(float x, float y, float z);

    int x() const { return m_x; }
    int y() const { return m_y; }
    int z() const { return m_z; }

    float dot(const Vector3& v);
    float dot(const Point3& p);

private:
    float m_x, m_y, m_z;
};
