#pragma once

#include <vector>

#include "intersection.h"
#include "material.h"
#include "shape.h"

class Ray;

class Model {
public:
    Model(std::vector<std::shared_ptr<Shape>> objects, Material material);

    Intersection testIntersect(const Ray &ray);
private:
    std::vector<std::shared_ptr<Shape>> m_objects;
    Material m_material;
};
