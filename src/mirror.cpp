#include "mirror.h"

Mirror::Mirror()
    : Material(Color(0.f))
{}

Color Mirror::f(
    const Intersection &intersection,
    const Vector3 &wo,
    float *pdf
) const {
    *pdf = 1.f;
    return Color(0.f);
}

Color Mirror::sampleF(
    const Intersection &intersection,
    RandomGenerator &random,
    Vector3 *wi,
    float *pdf
) const
{
    *pdf = 1.f;
    *wi = intersection.wi.reflect(intersection.normal);

    return Color(1.f);
}
