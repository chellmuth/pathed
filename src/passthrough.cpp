#include "passthrough.h"

#include "snell.h"
#include "tangent_frame.h"
#include "transform.h"
#include "world_frame.h"

#include <algorithm>
#include <cmath>
#include <iostream>

Passthrough::Passthrough()
    : Material(0.f)
{}

Color Passthrough::f(
    const Intersection &intersection,
    const Vector3 &wiWorld,
    float *pdf
) const
{
    *pdf = 0.f;
    return Color(0.f);
}

BSDFSample Passthrough::sample(
    const Intersection &intersection,
    RandomGenerator &random
) const
{
    const float cosTheta = WorldFrame::absCosTheta(-intersection.shadingNormal, -intersection.woWorld);

    BSDFSample sample = {
        .wiWorld = -intersection.woWorld,
        .pdf = 1.f,
        .throughput = Color(1.f) / cosTheta,
        .material = this
    };
    return sample;
}
