#include <stdio.h>
#include <math.h>

#include "vector.h"

#include "point.h"

Vector3::Vector3(float x, float y, float z)
    : m_x(x), m_y(y), m_z(z)
{}

float Vector3::dot(const Vector3& v)
{
    return m_x * v.x() + m_y * v.y() + m_z * v.z();
}

float Vector3::dot(const Point3& p)
{
    return m_x * p.x() + m_y * p.y() + m_z * p.z();
}

Vector3 Vector3::normalized()
{
    float norm = sqrt(
        m_x * m_x +
        m_y * m_y +
        m_z * m_z
    );

    return Vector3(
        m_x / norm,
        m_y / norm,
        m_z / norm
    );
}

Vector3 Vector3::operator* (const float t) const
{
    return Vector3(
        m_x * t,
        m_y * t,
        m_z * t
    );
}


void Vector3::debug()
{
    printf("<Vector3> (%f %f %f)\n", m_x, m_y, m_z);
}
