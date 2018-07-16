#include "scene.h"

#include "ray.h"
#include "util.h"

Scene::Scene(json sceneJson)
    : m_sphere(Sphere(sceneJson["sphere"]))
{}

Intersection Scene::testIntersect(const Ray &ray)
{
    return m_sphere.testIntersect(ray);
}
