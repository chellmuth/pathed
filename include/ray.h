#pragma once

#include "point.h"
#include "vector.h"

class Ray {
public:
    Ray(Point3 origin, Vector3 direction);

    const Point3& origin() const { return m_origin; }
    const Vector3& direction() const { return m_direction; }

    Point3 at(float t) const;

private:
    Point3 m_origin;
    Vector3 m_direction;
};
