#include "mirror.h"

#include "transform.h"

Mirror::Mirror()
    : Material(0.f)
{}

Color Mirror::f(
    const Intersection &intersection,
    const Vector3 &wo,
    float *pdf
) const
{
    *pdf = 0.f;
    return Color(0.f);
}

Vector3 Mirror::sample(
    const Intersection &intersection,
    RandomGenerator &random,
    float *pdf
) const
{
    *pdf = 1.f;

    Transform worldToTangent = worldSpaceToNormal(
        intersection.normal,
        intersection.wi
    );

    Vector3 localWi = worldToTangent.apply(intersection.wi);
    Vector3 localWo = localWi.reflect(Vector3(0.f, 1.f, 0.f));

    Transform tangentToWorld = normalToWorldSpace(
        intersection.normal,
        intersection.wi
    );

    return tangentToWorld.apply(localWo);
}
