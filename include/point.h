#pragma once

#include <sstream>
#include <string>

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
    std::string toString() const {
        std::ostringstream oss;
        oss << "[Point3 x=" << m_x << " y=" << m_y << " z=" << m_z << "]";
        return oss.str();
    }

private:
    float m_x, m_y, m_z;
};
