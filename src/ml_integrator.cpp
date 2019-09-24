#include "ml_integrator.h"

#include "globals.h"
#include "job.h"
#include "light.h"
#include "monte_carlo.h"
#include "ray.h"
#include "transform.h"
#include "util.h"
#include "vector.h"

#include <iostream>

Color MLIntegrator::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    Color result(0.f);
    if (m_bounceController.checkCounts(1)) {
        result = direct(intersection, scene, random);
    }

    Color modulation = Color(1.f);
    Intersection lastIntersection = intersection;

    for (int bounce = 2; !m_bounceController.checkDone(bounce); bounce++) {
        Transform hemisphereToWorld = normalToWorldSpace(
            lastIntersection.normal,
            lastIntersection.wi
        );

        Vector3 hemisphereSample = CosineSampleHemisphere(random);
        float pdf = CosineHemispherePdf(hemisphereSample);

        Vector3 bounceDirection = hemisphereToWorld.apply(hemisphereSample);
        Ray bounceRay(
            lastIntersection.point,
            bounceDirection
        );

        Intersection bounceIntersection = scene.testIntersect(bounceRay);
        if (!bounceIntersection.hit) { break; }

        Color f = lastIntersection.material->f(lastIntersection, bounceDirection);
        float invPDF = 1.f / pdf;

        modulation *= f
            * fmaxf(0.f, bounceDirection.dot(lastIntersection.normal))
            * invPDF;
        lastIntersection = bounceIntersection;

        if (m_bounceController.checkCounts(bounce)) {
            const Color previous = result;

            result += direct(bounceIntersection, scene, random) * modulation;
        }
    }

    return result;
}

Color MLIntegrator::direct(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random
) const {
    Color emit = intersection.material->emit();
    if (!emit.isBlack()) {
        // part of my old logic - if you hit an emitter, don't do direct lighting?
        return Color(0.f, 0.f, 0.f);
    }

    int lightCount = scene.lights().size();
    int lightIndex = (int)floorf(random.next() * lightCount);

    std::shared_ptr<Light> light = scene.lights()[lightIndex];
    SurfaceSample lightSample = light->sample(random);

    Vector3 lightDirection = (lightSample.point - intersection.point).toVector();
    Vector3 wo = lightDirection.normalized();

    if (lightSample.normal.dot(wo) >= 0.f) {
        return Color(0.f);
    }

    Ray shadowRay = Ray(intersection.point, wo);
    float lightDistance = lightDirection.length();
    bool occluded = scene.testOcclusion(shadowRay, lightDistance);

    if (occluded) {
        return Color(0.f);
    }

    float invPDF = lightSample.invPDF * lightCount;

    return light->biradiance(lightSample, intersection.point)
        * intersection.material->f(intersection, wo)
        * fmaxf(0.f, wo.dot(intersection.normal))
        * invPDF;
}
