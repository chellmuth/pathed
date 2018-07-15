#include "scene.h"

#include "util.h"

Scene::Scene(json sceneJson)
    : m_sphere(Sphere(sceneJson["sphere"]))
{}

bool Scene::testIntersect(const Ray &ray)
{
    return m_sphere.testIntersect(ray);
}
