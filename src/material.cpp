#include "material.h"

#include "intersection.h"

Material::Material(Color emit)
    : m_emit(emit)
{}

Color Material::emit() const
{
    return m_emit;
}

Vector3 Material::buildLocalWi(const Intersection &intersection) const
{
    // intersection.wo is wi from the brdf's perspective
    return intersection.worldToTangent.apply(intersection.woWorld).normalized();
}

std::tuple<Vector3, Vector3> Material::buildLocalWs(
    const Intersection &intersection,
    const Vector3 &woWorld
) const {
    // path tracing intersection has the directions flipped,
    // this returns (localWi, localWo)
    return std::make_tuple<Vector3, Vector3>(
        intersection.worldToTangent.apply(intersection.woWorld).normalized(),
        intersection.worldToTangent.apply(woWorld).normalized()
    );
}
