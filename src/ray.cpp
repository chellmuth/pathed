#include "ray.h"

Ray::Ray(Point3 origin, Vector3 direction)
    : m_origin(origin), m_direction(direction)
{}

Point3 Ray::at(float t) const
{
    return m_origin + m_direction * t;
}
