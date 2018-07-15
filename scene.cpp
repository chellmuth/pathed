#include "scene.h"

#include "util.h"

Scene::Scene()
{
}

bool Scene::testIntersect(const Ray &ray)
{
    Point3 center(120.f, 200.f, 100.f);
    float radius = 100.f;

    Point3 origin = ray.origin();
    Vector3 direction = ray.direction();

    Point3 L = origin - center;

    float a = direction.dot(direction);
    float b = 2 * direction.dot(L);
    float c = L.dot(L) - radius * radius;

    QuadraticSolution solution = solveQuadratic(a, b, c);
    return solution.hasRealSolutions;
}
