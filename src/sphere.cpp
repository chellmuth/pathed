#include <limits>

#include "sphere.h"

#include "color.h"
#include "ray.h"
#include "util.h"

Sphere::Sphere(Point3 center, float radius, Color color)
    : m_center(center), m_radius(radius), m_color(color)
{}

Intersection Sphere::testIntersect(const Ray &ray)
{
    Point3 origin = ray.origin();
    Vector3 direction = ray.direction();

    Point3 L = origin - m_center;

    float a = direction.dot(direction);
    float b = 2 * direction.dot(L);
    float c = L.dot(L) - m_radius * m_radius;

    QuadraticSolution solution = solveQuadratic(a, b, c);
    if (solution.hasRealSolutions) {
        Point3 hitPoint = ray.at(solution.solution1);
        Intersection result = {
            .hit = true,
            .t = solution.solution1,
            .normal = (hitPoint - m_center).toVector().normalized(),
            .color = m_color
        };
        return result;
    } else {
        Intersection result = {
            .hit = false,
            .t = std::numeric_limits<float>::max(),
            .normal = Vector3(0.f, 0.f, 0.f),
            .color = Color(0.f, 0.f, 0.f)
        };
        return result;
    }
}
