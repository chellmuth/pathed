#include "ray.h"

#include <stdio.h>

Ray::Ray(Point3 origin, Vector3 direction)
    : m_origin(origin), m_direction(direction)
{}

Point3 Ray::at(float t) const
{
    return m_origin + m_direction * t;
}

void Ray::debug() const
{
    printf(
        "<Ray> [Origin: %f %f %f] [Direction: %f %f %f]\n",
        m_origin.x(), m_origin.y(), m_origin.z(),
        m_direction.x(), m_direction.y(), m_direction.z()
    );
}
