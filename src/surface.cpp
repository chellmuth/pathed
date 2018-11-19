#include "surface.h"

#include "ray.h"

Surface::Surface(std::shared_ptr<Shape> shape, std::shared_ptr<Material> material)
    : m_shape(shape), m_material(material)
{}

SurfaceSample Surface::sample(RandomGenerator &random) const
{
    return m_shape->sample(random);
}

Intersection Surface::testIntersect(const Ray &ray) const
{
    Intersection intersection = m_shape->testIntersect(ray);
    intersection.material = m_material.get();

    return intersection;
}

std::shared_ptr<Shape> Surface::getShape() const
{
    return m_shape;
}
