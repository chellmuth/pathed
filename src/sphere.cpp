#include <limits>

#include "sphere.h"

#include "color.h"
#include "ray.h"
#include "util.h"

Sphere::Sphere(Point3 center, float radius, Color color)
    : m_center(center), m_radius(radius), m_color(color)
{}

SurfaceSample Sphere::sample(RandomGenerator &random) const
{
    float theta = random.next() * TWO_PI;
    float phi = random.next() * TWO_PI;

    Vector3 v = Vector3(
        m_radius * cosf(theta) * sinf(phi),
        m_radius * sinf(theta) * sinf(phi),
        m_radius * cosf(phi)
    );

    SurfaceSample sample = {
        .point = m_center + v,
        .normal = v.normalized(),
        .invPDF = area()
    };
    return sample;
}

Intersection Sphere::testIntersect(const Ray &ray)
{
    Point3 origin = ray.origin();
    Vector3 direction = ray.direction();

    Point3 L = origin - m_center;

    float a = direction.dot(direction);
    float b = 2 * direction.dot(L);
    float c = L.dot(L) - m_radius * m_radius;

    QuadraticSolution solution = solveQuadratic(a, b, c);
    if (solution.hasRealSolutions && solution.solution1 > 0) {
        Point3 hitPoint = ray.at(solution.solution1);
        Intersection result = {
            .hit = true,
            .t = solution.solution1,
            .point = hitPoint,
            .wi = ray.direction(),
            .normal = (hitPoint - m_center).toVector().normalized(),
            .material = nullptr
        };
        return result;
    } else {
        Intersection result = {
            .hit = false,
            .t = std::numeric_limits<float>::max(),
            .point = Point3(0.f, 0.f, 0.f),
            .wi = Vector3(0.f),
            .normal = Vector3(0.f),
            .material = nullptr
        };
        return result;
    }
}

float Sphere::area() const
{
    return 4 * M_PI * m_radius * m_radius;
}
