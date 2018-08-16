#pragma once

#include <memory>
#include <vector>

#include "intersection.h"
#include "light.h"
#include "point.h"
#include "surface.h"

class Ray;

class Scene {
public:
    Scene(std::vector<std::shared_ptr<Surface>> surfaces, std::vector<std::shared_ptr<Light>> lights);

    std::vector<std::shared_ptr<Light>> lights() const { return m_lights; }
    Intersection testIntersect(const Ray &ray) const;

private:
    std::vector<std::shared_ptr<Surface>> m_surfaces;
    std::vector<std::shared_ptr<Light>> m_lights;
};
