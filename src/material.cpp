#include "material.h"

#include <math.h>
#include <stdio.h>

#include "point.h"
#include "vector.h"

Color shade(const Intersection &intersection, const Scene &scene)
{
    Point3 light = scene.light();

    Vector3 lightDirection = (light - intersection.point).toVector().normalized();
    float LDotNormal = lightDirection.dot(intersection.normal);

    return intersection.color * fmaxf(0.f, LDotNormal);
}
