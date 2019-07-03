#include "math.h"
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
    // from pbrt
    float z = 1 - 2 * random.next();
    float r = sqrt(fmaxf(0, 1 - z * z));
    float phi = 2 * M_PI * random.next();
    Vector3 v(r * cosf(phi), r * sinf(phi), z);

    SurfaceSample sample = {
        .point = m_center + v * m_radius,
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
            .uv = { 0.f, 0.f },
            .material = nullptr
        };
        return result;
    } else {
        Intersection result = IntersectionHelper::miss;
    }
}

float Sphere::area() const
{
    return 4 * M_PI * m_radius * m_radius;
}

void Sphere::updateAABB(AABB *aabb)
{
    Point3 bottomLeftFront(-m_radius, -m_radius, -m_radius);
    Point3 topRightBack(m_radius, m_radius, m_radius);

    aabb->update(m_center + bottomLeftFront);
    aabb->update(m_center + topRightBack);
}

std::shared_ptr<Shape> Sphere::transform(const Transform &transform) const
{
    throw "Sphere transform unimplemented";
}
