#include "material.h"

#include <math.h>
#include <stdio.h>
#include <iostream>

#include "color.h"
#include "intersection.h"
#include "point.h"
#include "ray.h"
#include "scene.h"
#include "vector.h"

Material::Material(Color diffuse, Color emit)
    : m_diffuse(diffuse), m_emit(emit)
{}

Color Material::shade(const Intersection &intersection, const Scene &scene, RandomGenerator &random) const
{
    float whichLight = random.next();
    int lightIndex = whichLight > 0.5f ? 0 : 1;

    Point3 light = scene.lights()[lightIndex]->sample(random);

    Vector3 lightDirection = (light - intersection.point).toVector();
    float lightDistance = lightDirection.length();

    Vector3 normalizedLightDirection = lightDirection.normalized();

    Ray shadowRay = Ray(intersection.point, normalizedLightDirection);
    Intersection shadowIntersection = scene.testIntersect(shadowRay);
    if (shadowIntersection.hit && shadowIntersection.t + 0.001f < lightDistance) {
        return Color(0.f, 0.f, 0.f);
    }

    float lightDotNormal = normalizedLightDirection.dot(intersection.normal);
    return m_diffuse
        * fmaxf(0.f, lightDotNormal)
        * (1.0 / (lightDistance * lightDistance))
        * scene.lights().size();
}
