#pragma once

#include <list>

#include "intersection.h"
#include "sphere.h"

class Ray;

class Scene {
public:
    Scene(std::list<Sphere> objects);

    Intersection testIntersect(const Ray &ray);

private:
    std::list<Sphere> m_objects;
};
