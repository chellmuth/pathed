#pragma once

#include <vector>

#include "intersection.h"
#include "point.h"
#include "shape.h"

class Ray;

class Scene {
public:
    Scene(std::vector<Shape *> objects, Point3 light);

    Point3 light() const { return m_light; }
    Intersection testIntersect(const Ray &ray);

private:
    std::vector<Shape *> m_objects;
    Point3 m_light;
};
