#include "scene.h"

#include "color.h"
#include "ray.h"
#include "util.h"

Scene::Scene(std::list<Sphere> objects)
    : m_objects(objects)
{}

Intersection Scene::testIntersect(const Ray &ray)
{
    Intersection result = {
        .hit = false,
        .t = std::numeric_limits<float>::max(),
        .normal = Vector3(0.f, 0.f, 0.f),
        .color = Color(0.f, 0.f, 0.f)
    };

    for (Sphere sphere : m_objects) {
        Intersection intersection = sphere.testIntersect(ray);
        if (intersection.hit && intersection.t < result.t) {
            result = intersection;
        }
    }

    return result;
}
