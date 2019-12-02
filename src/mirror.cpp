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

BSDFSample Mirror::sample(
    const Intersection &intersection,
    RandomGenerator &random
) const
{
    Transform worldToTangent = worldSpaceToNormal(
        intersection.shadingNormal,
        intersection.wi
    );

    Vector3 localWi = worldToTangent.apply(intersection.wi);
    Vector3 localWo = localWi.reflect(Vector3(0.f, 1.f, 0.f));

    Transform tangentToWorld = normalToWorldSpace(
        intersection.shadingNormal,
        intersection.wi
    );

    BSDFSample sample = {
        .wi = tangentToWorld.apply(localWo),
        .pdf = 1.f,
        .throughput = Color(1.f) / localWo.y()
    };

    return sample;
}
