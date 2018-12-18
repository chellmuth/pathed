#include "surface.h"

#include "ray.h"

Surface::Surface(
    std::shared_ptr<Shape> shape,
    std::shared_ptr<Material> material,
    Color radiance
)
    : m_shape(shape), m_material(material), m_radiance(radiance)
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

std::shared_ptr<Material> Surface::getMaterial() const
{
    return m_material;
}

Color Surface::getRadiance() const
{
    return m_radiance;
}
