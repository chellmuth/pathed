#include "vector.h"

#include "point.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>


Vector3::Vector3(float x, float y, float z)
    : m_x(x), m_y(y), m_z(z)
{
    assert(!std::isnan(m_x));
    assert(!std::isnan(m_y));
    assert(!std::isnan(m_z));
}

float Vector3::dot(const Vector3& v) const
{
    return m_x * v.x() + m_y * v.y() + m_z * v.z();
}

float Vector3::dot(const Point3& p) const
{
    return m_x * p.x() + m_y * p.y() + m_z * p.z();
}

float Vector3::absDot(const Vector3& v) const
{
    return std::abs(m_x * v.x() + m_y * v.y() + m_z * v.z());
}

float Vector3::length() const
{
    return sqrtf(
        m_x * m_x +
        m_y * m_y +
        m_z * m_z
    );
}

Vector3 Vector3::cross(const Vector3& v) const
{
    return Vector3(
        (m_y * v.z()) - (m_z * v.y()),
        (m_z * v.x()) - (m_x * v.z()),
        (m_x * v.y()) - (m_y * v.x())
    );
}

Vector3 Vector3::normalized() const
{
    const float norm = sqrt(
        m_x * m_x +
        m_y * m_y +
        m_z * m_z
    );

    assert(norm > 0.f);

    const Vector3 normalized = Vector3(
        m_x / norm,
        m_y / norm,
        m_z / norm
    );
    return normalized;
}

Vector3 Vector3::reflect(const Vector3& normal) const
{
    return (normal * dot(normal) * 2) - *this;
}

Vector3 Vector3::operator* (const float t) const
{
    return Vector3(
        m_x * t,
        m_y * t,
        m_z * t
    );
}

Vector3 Vector3::operator- (const Vector3& v) const
{
    return Vector3(
        m_x - v.x(),
        m_y - v.y(),
        m_z - v.z()
    );
}

Vector3 Vector3::operator+ (const Vector3& v) const
{
    return Vector3(
        m_x + v.x(),
        m_y + v.y(),
        m_z + v.z()
    );
}

Vector3 Vector3::operator- () const
{
    return Vector3(
        -m_x,
        -m_y,
        -m_z
    );
}

Vector3 Vector3::operator/ (const Vector3& v) const
{
    return Vector3(
        m_x / v.x(),
        m_y / v.y(),
        m_z / v.z()
    );
}

bool Vector3::operator==(const Vector3 &v) const
{
    return m_x == v.x() && m_y == v.y() && m_z == v.z();
}

void Vector3::debug() const
{
    printf("<Vector3> (%f %f %f)\n", m_x, m_y, m_z);
}
