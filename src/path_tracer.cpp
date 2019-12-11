#include "path_tracer.h"

#include "bounce_controller.h"
#include "color.h"
#include "globals.h"
#include "job.h"
#include "light.h"
#include "mis.h"
#include "monte_carlo.h"
#include "ray.h"
#include "transform.h"
#include "util.h"
#include "vector.h"

#include "json.hpp"
using json = nlohmann::json;

#include <fstream>
#include <iostream>

Color PathTracer::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    sample.eyePoints.push_back(intersection.point);

    BSDFSample bsdfSample = intersection.material->sample(intersection, random);

    Color result(0.f);
    if (m_bounceController.checkCounts(1)) {
        result = direct(intersection, bsdfSample, scene, random, sample);
        sample.contributions.push_back({result, 1.f});
    }

    Color modulation = Color(1.f);
    Intersection lastIntersection = intersection;

    for (int bounce = 2; !m_bounceController.checkDone(bounce); bounce++) {
        Ray bounceRay(lastIntersection.point, bsdfSample.wi);

        Intersection bounceIntersection = scene.testIntersect(bounceRay);
        if (!bounceIntersection.hit) { break; }

        sample.eyePoints.push_back(bounceIntersection.point);

        const float invPDF = 1.f / bsdfSample.pdf;
        modulation *= bsdfSample.throughput
            * fmaxf(0.f, bsdfSample.wi.dot(lastIntersection.shadingNormal))
            * invPDF;

        bsdfSample = bounceIntersection.material->sample(
            bounceIntersection, random
        );

        lastIntersection = bounceIntersection;

        if (m_bounceController.checkCounts(bounce)) {
            const Color previous = result;

            Color Ld = direct(bounceIntersection, bsdfSample, scene, random, sample);
            result += Ld * modulation;

            sample.contributions.push_back({result - previous, invPDF});
        }
    }

    return result;
}

Color PathTracer::direct(
    const Intersection &intersection,
    const BSDFSample &bsdfSample,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    Color emit = intersection.material->emit();
    if (!emit.isBlack()) {
        // part of my old logic - if you hit an emitter, don't do direct lighting?
        return Color(0.f, 0.f, 0.f);
    }

    int lightCount = scene.lights().size();
    int lightIndex = (int)floorf(random.next() * lightCount);

    std::shared_ptr<Light> light = scene.lights()[lightIndex];
    SurfaceSample lightSample = light->sample(intersection, random);

    Vector3 lightDirection = (lightSample.point - intersection.point).toVector();
    Vector3 wo = lightDirection.normalized();

    if (lightSample.normal.dot(wo) >= 0.f) {
        sample.shadowTests.push_back({
            intersection.point,
            lightSample.point,
            true
        });

        return Color(0.f);
    }

    Ray shadowRay = Ray(intersection.point, wo);
    float lightDistance = lightDirection.length();
    bool occluded = scene.testOcclusion(shadowRay, lightDistance);

    sample.shadowTests.push_back({
        intersection.point,
        lightSample.point,
        occluded
    });

    Color result(0.f);

    const float invPDF = lightSample.invPDF * lightCount;

    if (!occluded) {
        const float brdfPDF = bsdfSample.material->pdf(intersection, wo);
        const float lightWeight = MIS::balanceWeight(1, 1, 1.f / invPDF, 0.f);

        const Color lightContribution = light->biradiance(lightSample, intersection.point)
            * lightWeight
            * intersection.material->f(intersection, wo)
            * fmaxf(0.f, wo.dot(intersection.shadingNormal))
            * invPDF;

        result += lightContribution;
    }

    const Ray bounceRay(intersection.point, bsdfSample.wi);
    Intersection bounceIntersection = scene.testIntersect(bounceRay);
    if (bounceIntersection.hit && bounceIntersection.isEmitter()) {
        const float lightPDF = 1.f / invPDF;
        const float brdfWeight = MIS::balanceWeight(1, 1, bsdfSample.pdf, lightPDF);

        const Color brdfContribution = bounceIntersection.material->emit()
            * brdfWeight
            * intersection.material->f(intersection, bsdfSample.wi)
            * fmaxf(0.f, bsdfSample.wi.dot(intersection.shadingNormal))
            * (1.f / bsdfSample.pdf);

        result += brdfContribution;
    }

    return result;
}
