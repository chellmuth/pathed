#pragma once

#include <ostream>

class Vector3;

class Point3 {
public:
    Point3(float x, float y, float z);

    float x() const { return m_x; }
    float y() const { return m_y; }
    float z() const { return m_z; }

    float dot(const Point3& p) const;

    Vector3 toVector() const;

    Point3 operator* (const float t) const;
    Point3 operator- (const Point3& v) const;
    Point3 operator+ (const Point3& v) const;
    Point3 operator+ (const Vector3& v) const;

    void debug() const;

private:
    float m_x, m_y, m_z;
};

std::ostream &operator<<(std::ostream &os, const Point3 &p);
