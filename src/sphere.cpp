#include "sphere.h"

#include "util.h"

Sphere::Sphere(json sphereJson)
    : m_center(sphereJson["center"]), m_radius(sphereJson["radius"])
{}

Sphere::Sphere(Point3 center, float radius)
    : m_center(center), m_radius(radius)
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
            .normal = (hitPoint - m_center).toVector().normalized()
        };
        return result;
    } else {
        Intersection result = {
            .hit = false,
            .normal = Vector3(0.f, 0.f, 0.f)
        };
        return result;
    }
}
