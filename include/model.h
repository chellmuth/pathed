#pragma once

#include <vector>

#include "intersection.h"
#include "material.h"
#include "shape.h"

class Ray;

class Model {
public:
    Model(std::vector<Shape *> objects, Material material);

    Intersection testIntersect(const Ray &ray);
private:
    std::vector<Shape *> m_objects;
    Material m_material;
};
