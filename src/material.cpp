#include "material.h"

#include <math.h>
#include <stdio.h>

#include "point.h"
#include "vector.h"

Color shade(const Intersection &intersection)
{
    Point3 fakeLight = Point3(
        0.f, 0.f, 0.f
    );

    Vector3 lightDirection = (fakeLight - intersection.point).toVector().normalized();
    float LDotNormal = lightDirection.dot(intersection.normal);

    return intersection.color * fmaxf(0.f, LDotNormal);
}
