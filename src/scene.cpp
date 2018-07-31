#include <limits>

#include "scene.h"

#include "color.h"
#include "ray.h"
#include "util.h"

Scene::Scene(std::vector<std::shared_ptr<Surface>> surfaces, Point3 light)
    : m_surfaces(surfaces), m_light(light)
{}

Intersection Scene::testIntersect(const Ray &ray) const
{
    Intersection result = {
        .hit = false,
        .t = std::numeric_limits<float>::max(),
        .point = Point3(0.f, 0.f, 0.f),
        .normal = Vector3(),
        .material = nullptr
    };

    for (std::shared_ptr<Surface> surfacePtr : m_surfaces) {
        Intersection intersection = surfacePtr.get()->testIntersect(ray);
        if (intersection.hit && intersection.t < result.t) {
            result = intersection;
        }
    }

    return result;
}
