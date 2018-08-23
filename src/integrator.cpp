#include "integrator.h"

#include "color.h"
#include "light.h"
#include "monte_carlo.h"
#include "ray.h"
#include "transform.h"
#include "vector.h"

Color Integrator::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    int bounceCount) const
{
    Color result = direct(intersection, scene, random);

    Color modulation = Color(1.f, 1.f, 1.f);
    Intersection lastIntersection = intersection;

    for (int bounce = 0; bounce < bounceCount; bounce++) {
        Transform hemisphereToWorld = normalToWorldSpace(
            lastIntersection.normal,
            lastIntersection.wi
        );

        Vector3 hemisphereSample = UniformSampleHemisphere(random);
        Vector3 bounceDirection = hemisphereToWorld.apply(hemisphereSample);
        Ray bounceRay(
            lastIntersection.point,
            bounceDirection
        );

        Intersection bounceIntersection = scene.testIntersect(bounceRay);
        if (!bounceIntersection.hit) { break; }

        float invPDF = 1.f / UniformHemispherePdf();

        modulation *= lastIntersection.material->f(lastIntersection.wi, bounceDirection)
            * fmaxf(0.f, bounceDirection.dot(lastIntersection.normal))
            * invPDF;
        lastIntersection = bounceIntersection;

        result += direct(bounceIntersection, scene, random) * modulation;
    }

    return result;
}

Color Integrator::direct(const Intersection &intersection, const Scene &scene, RandomGenerator &random) const
{
    Color emit = intersection.material->emit();
    if (!emit.isBlack()) {
        return Color(0.f, 0.f, 0.f);
    }

    int lightCount = scene.lights().size();
    int lightIndex = (int)floorf(random.next() * lightCount);

    std::shared_ptr<Light> light = scene.lights()[lightIndex];
    SurfaceSample lightSample = light->sample(random);

    Vector3 lightDirection = (lightSample.point - intersection.point).toVector();
    Vector3 wo = lightDirection.normalized();

    if (lightSample.normal.dot(wo) >= 0.f) {
        return Color(0.f, 0.f, 0.f);
    }

    Ray shadowRay = Ray(intersection.point, wo);
    Intersection shadowIntersection = scene.testIntersect(shadowRay);
    float lightDistance = lightDirection.length();

    if (shadowIntersection.hit && shadowIntersection.t + 0.0001f < lightDistance) {
        return Color(0.f, 0.f, 0.f);
    }

    float invPDF = lightSample.invPDF * lightCount;

    return light->biradiance(lightSample.point, intersection.point)
        * intersection.material->f(intersection.wi, wo)
        * fmaxf(0.f, wo.dot(intersection.normal))
        * invPDF;
}
