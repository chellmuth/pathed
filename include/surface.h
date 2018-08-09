#pragma once

#include <vector>

#include "intersection.h"
#include "material.h"
#include "random_generator.h"
#include "shape.h"

class Ray;

class Surface {
public:
    Surface(std::shared_ptr<Shape> shape, std::shared_ptr<Material> material);

    Point3 sample(RandomGenerator &random) const;
    Intersection testIntersect(const Ray &ray) const;
private:
    std::shared_ptr<Shape> m_shape;
    std::shared_ptr<Material> m_material;
};
