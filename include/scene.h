#pragma once

#include <memory>
#include <vector>

#include "intersection.h"
#include "light.h"
#include "point.h"
#include "surface.h"

class Camera;
class Model;
class Ray;

class Scene {
public:
    Scene(
        // old
        std::vector<std::shared_ptr<Surface>> surfaces,
        std::vector<std::shared_ptr<Light>> lights,

        // new
        std::vector<std::shared_ptr<Model>> models,
        std::shared_ptr<Camera> camera
    );

    std::vector<std::shared_ptr<Light>> lights() const { return m_lights; }
    Intersection testIntersect(const Ray &ray) const;

    std::vector<std::shared_ptr<Surface>> getSurfaces();

private:
    std::vector<std::shared_ptr<Surface>> m_surfaces;
    std::vector<std::shared_ptr<Light>> m_lights;
    std::vector<std::shared_ptr<Model>> m_models;
    std::shared_ptr<Camera> m_camera;
};
