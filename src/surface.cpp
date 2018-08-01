#include "surface.h"

#include "ray.h"

Surface::Surface(std::shared_ptr<Shape> shape, std::shared_ptr<Material> material)
    : m_shape(shape), m_material(material)
{}

Intersection Surface::testIntersect(const Ray &ray)
{
    Intersection intersection = m_shape.get()->testIntersect(ray);
    intersection.material = m_material.get();

    return intersection;
}

