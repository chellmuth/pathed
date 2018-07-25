#include "material.h"

#include <math.h>
#include <stdio.h>

#include "color.h"
#include "intersection.h"
#include "point.h"
#include "ray.h"
#include "vector.h"

Color shade(const Intersection &intersection, const Scene &scene)
{
    Point3 light = scene.light();

    Vector3 lightDirection = (light - intersection.point).toVector();
    float lightDistance = lightDirection.length();

    Vector3 normalizedLightDirection = lightDirection.normalized();

    Ray shadowRay = Ray(intersection.point, normalizedLightDirection);
    Intersection shadowIntersection = scene.testIntersect(shadowRay);
    if (shadowIntersection.hit && shadowIntersection.t < lightDistance) {
        return Color(0.f, 0.f, 0.f);
    }

    float lightDotNormal = normalizedLightDirection.dot(intersection.normal);
    return intersection.color * fmaxf(0.f, lightDotNormal);
}
