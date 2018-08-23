#pragma once

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

    Vector3 normalized() const;

    Vector3 operator* (const float t) const;
    Vector3 operator- (const Vector3& v) const;
    bool operator==(const Vector3 &v) const;

    void debug() const;

private:
    float m_x, m_y, m_z;
};
