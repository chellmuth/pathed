#include "scene.h"

#include "camera.h"
#include "color.h"
#include "ray.h"
#include "util.h"

#include <limits>

Scene::Scene(
    std::vector<std::shared_ptr<Surface>> surfaces,
    std::vector<std::shared_ptr<Light>> lights,
    std::shared_ptr<Camera> camera
)
    : m_surfaces(surfaces), m_lights(lights), m_camera(camera), m_bvh(new BVH())
{
    std::vector<std::shared_ptr<Primitive>> primitives(m_surfaces.begin(), m_surfaces.end());
    m_bvh->bake(primitives);
}

std::vector<std::shared_ptr<Surface>> Scene::getSurfaces()
{
    return m_surfaces;
}

std::shared_ptr<Camera> Scene::getCamera()
{
    return m_camera;
}

Intersection Scene::testIntersect(const Ray &ray) const
{
    Intersection result = {
        .hit = false,
        .t = std::numeric_limits<float>::max(),
        .point = Point3(0.f, 0.f, 0.f),
        .wi = Vector3(0.f),
        .normal = Vector3(0.f),
        .material = nullptr
    };

    for (std::shared_ptr<Surface> surfacePtr : m_surfaces) {
        Intersection intersection = surfacePtr->testIntersect(ray);
        if (intersection.hit && intersection.t < result.t) {
            result = intersection;
        }
    }

    return result;
}
