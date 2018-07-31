#pragma once

#include <vector>

#include "intersection.h"
#include "surface.h"
#include "point.h"

class Ray;

class Scene {
public:
    Scene(std::vector<std::shared_ptr<Surface>> surfaces, Point3 light);

    Point3 light() const { return m_light; }
    Intersection testIntersect(const Ray &ray) const;

private:
    std::vector<std::shared_ptr<Surface>> m_surfaces;
    Point3 m_light;
};
