#include <limits>

#include "scene.h"

#include "color.h"
#include "ray.h"
#include "util.h"

Scene::Scene(std::vector<Shape *> objects, Point3 light)
    : m_objects(objects), m_light(light)
{}

Intersection Scene::testIntersect(const Ray &ray) const
{
    Intersection result = {
        .hit = false,
        .t = std::numeric_limits<float>::max(),
        .point = Point3(0.f, 0.f, 0.f),
        .normal = Vector3(),
        .color = Color(0.f, 0.f, 0.f)
    };

    for (Shape *shape : m_objects) {
        Intersection intersection = shape->testIntersect(ray);
        if (intersection.hit && intersection.t < result.t) {
            result = intersection;
        }
    }

    return result;
}
