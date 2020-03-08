#include "surface.h"

#include "ray.h"

Surface::Surface(
    std::shared_ptr<Shape> shape,
    std::shared_ptr<Material> material,
    std::shared_ptr<Medium> internalMedium,
    int faceIndex
) : m_shape(shape),
    m_material(material),
    m_internalMedium(internalMedium),
    m_faceIndex(faceIndex)
{}

Surface::Surface(
    std::shared_ptr<Shape> shape,
    std::shared_ptr<Material> material,
    std::shared_ptr<Medium> internalMedium
) : Surface(shape, material, internalMedium, 0)
{}

SurfaceSample Surface::sample(RandomGenerator &random) const
{
    return m_shape->sample(random);
}

SurfaceSample Surface::sample(const Point3 &referencePoint, RandomGenerator &random) const
{
    return m_shape->sample(referencePoint, random);
}

float Surface::pdf(const Point3 &point, Measure measure) const
{
    return m_shape->pdf(point, measure);
}

float Surface::pdf(const Point3 &point, const Point3 &referencePoint, Measure measure) const
{
    return m_shape->pdf(point, referencePoint, measure);
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
    return m_material->emit();
}
