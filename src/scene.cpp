#include "scene.h"

#include "ray.h"
#include "util.h"

Scene::Scene(json sceneJson)
{
    auto objects = sceneJson["objects"];
    for (json::iterator it = objects.begin(); it != objects.end(); ++it) {
        m_objects.push_back(Sphere((*it)["parameters"]));
    }
}

Intersection Scene::testIntersect(const Ray &ray)
{
    for (Sphere sphere : m_objects) {
        Intersection intersection = sphere.testIntersect(ray);
        if (intersection.hit) {
            return intersection;
        }
    }

    Intersection miss = { .hit = false, .normal = Vector3(0.f, 0.f, 0.f) };
    return miss;
}
