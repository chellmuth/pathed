#pragma once

#include <vector>

#include "intersection.h"
#include "model.h"
#include "point.h"

class Ray;

class Scene {
public:
    Scene(std::vector<Model> models, Point3 light);

    Point3 light() const { return m_light; }
    Intersection testIntersect(const Ray &ray) const;

private:
    std::vector<Model> m_models;
    Point3 m_light;
};
