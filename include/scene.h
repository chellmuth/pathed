#pragma once

#include <memory>
#include <vector>

#include "bvh.h"
#include "intersection.h"
#include "light.h"
#include "point.h"
#include "primitive.h"
#include "surface.h"

class Camera;
class Ray;

class Scene {
public:
    Scene(
        // old
        std::vector<std::shared_ptr<Surface>> surfaces,
        std::vector<std::shared_ptr<Light>> lights,

        // new
        std::shared_ptr<Camera> camera
    );

    std::vector<std::shared_ptr<Light>> lights() const { return m_lights; }
    Intersection testIntersect(const Ray &ray) const;

    std::vector<std::shared_ptr<Surface>> getSurfaces();
    std::shared_ptr<Camera> getCamera();

private:
    std::unique_ptr<BVH> m_bvh;
    std::vector<std::shared_ptr<Surface>> m_surfaces;
    std::vector<std::shared_ptr<Light>> m_lights;
    std::shared_ptr<Camera> m_camera;
};
