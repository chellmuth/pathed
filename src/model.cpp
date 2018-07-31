#include "model.h"

#include "ray.h"

Model::Model(std::vector<Shape *> objects, Material material)
    : m_objects(objects), m_material(material)
{}

Intersection Model::testIntersect(const Ray &ray)
{
    Intersection result = {
        .hit = false,
        .t = std::numeric_limits<float>::max(),
        .point = Point3(0.f, 0.f, 0.f),
        .normal = Vector3(),
        .material = nullptr
    };

    for (Shape *shape : m_objects) {
        Intersection intersection = shape->testIntersect(ray);
        if (intersection.hit && intersection.t < result.t) {
            result = intersection;
            result.material = &this->m_material;
        }
    }

    return result;
}
