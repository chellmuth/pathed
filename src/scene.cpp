#include <limits>

#include "scene.h"

#include "color.h"
#include "ray.h"
#include "util.h"

Scene::Scene(std::vector<std::shared_ptr<Model>> models, Point3 light)
    : m_models(models), m_light(light)
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

    for (std::shared_ptr<Model> modelPtr : m_models) {
        Intersection intersection = modelPtr.get()->testIntersect(ray);
        if (intersection.hit && intersection.t < result.t) {
            result = intersection;
        }
    }

    return result;
}
