#pragma once

class Point3;

class Vector3 {
public:
    Vector3(float x, float y, float z);

    float x() const { return m_x; }
    float y() const { return m_y; }
    float z() const { return m_z; }

    float dot(const Vector3& v);
    float dot(const Point3& p);

    Vector3 normalized();

    Vector3 operator* (const float t) const;

    void debug();

private:
    float m_x, m_y, m_z;
};
