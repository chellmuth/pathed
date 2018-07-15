#include "vector.h"

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
