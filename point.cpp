#include <stdio.h>

#include "point.h"

Point3::Point3(json pointJson)
{
    m_x = pointJson["x"];
    m_y = pointJson["y"];
    m_z = pointJson["z"];
}

Point3::Point3(float x, float y, float z)
    : m_x(x), m_y(y), m_z(z)
{}

float Point3::dot(const Point3& p)
{
    return m_x * p.x() + m_y * p.y() + m_z * p.z();
}

Point3 Point3::operator- (const Point3& p) const {
    return Point3(
        m_x - p.x(),
        m_y - p.y(),
        m_z - p.z()
    );
}

void Point3::debug()
{
    printf("<Point3> (%f %f %f)\n", m_x, m_y, m_z);
}
