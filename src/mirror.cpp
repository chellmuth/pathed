#include "mirror.h"

#include "transform.h"

#include <cmath>

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
        intersection.woWorld
    );

    Vector3 localWo = worldToTangent.apply(intersection.woWorld);
    Vector3 localWi = localWo.reflect(Vector3(0.f, 1.f, 0.f));

    BSDFSample sample = {
        .wi = intersection.tangentToWorld.apply(localWi),
        .pdf = 1.f,
        .throughput = Color(std::max(0.f, 1.f / localWi.y()))
    };

    return sample;
}
