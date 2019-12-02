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
        intersection.wo
    );

    Vector3 localWo = worldToTangent.apply(intersection.wo);
    Vector3 localWi = localWo.reflect(Vector3(0.f, 1.f, 0.f));

    Transform tangentToWorld = normalToWorldSpace(
        intersection.shadingNormal,
        intersection.wo
    );

    BSDFSample sample = {
        .wi = tangentToWorld.apply(localWi),
        .pdf = 1.f,
        .throughput = Color(1.f) / localWi.y()
    };

    return sample;
}
