#include <limits>

#include "triangle.h"

#include "point.h"
#include "ray.h"
#include "vector.h"

Triangle::Triangle(Point3 v0, Point3 v1, Point3 v2)
    : m_v0(v0), m_v1(v1), m_v2(v2)
{}

Intersection Triangle::testIntersect(const Ray &ray)
{
    Intersection miss = {
        .hit = false,
        .t = std::numeric_limits<float>::max(),
        .point = Point3(0.f, 0.f, 0.f),
        .normal = Vector3(0.f, 0.f, 0.f),
        .color = Color(0.f, 0.f, 0.f)
    };

    return miss;
}
