#pragma once

#include <algorithm>
#include <sstream>
#include <string>

class Point3;

class Vector3 {
public:
    Vector3(float xyz) : Vector3(xyz, xyz, xyz) {};
    Vector3(float x, float y, float z);

    float x() const { return m_x; }
    float y() const { return m_y; }
    float z() const { return m_z; }

    float dot(const Vector3& v) const;
    float dot(const Point3& p) const;
    float length() const;
    Vector3 cross(const Vector3& v) const;
    Vector3 reflect(const Vector3& normal) const;
    bool isZero() const { return m_x == 0.f && m_y == 0.f && m_z == 0.f; }

    Vector3 normalized() const;

    Vector3 operator* (const float t) const;
    Vector3 operator- (const Vector3& v) const;
    Vector3 operator+ (const Vector3& v) const;
    Vector3 operator/ (const Vector3& v) const;
    Vector3 operator- () const;
    bool operator==(const Vector3 &v) const;

    float min() const { return std::min(m_x, std::min(m_y, m_z)); }

    void debug() const;
    std::string toString() const {
        std::ostringstream oss;
        oss << "[Vector3 x=" << m_x << " y=" << m_y << " z=" << m_z << "]";
        return oss.str();
    }

private:
    float m_x, m_y, m_z;
};
