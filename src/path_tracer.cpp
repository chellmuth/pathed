#include "path_tracer.h"

#include "bounce_controller.h"
#include "color.h"
#include "globals.h"
#include "job.h"
#include "light.h"
#include "measure.h"
#include "mis.h"
#include "monte_carlo.h"
#include "ray.h"
#include "transform.h"
#include "util.h"
#include "vector.h"

#include <fstream>
#include <iostream>

Color PathTracer::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    int pixelIndex,
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
        Ray bounceRay(lastIntersection.point, bsdfSample.wiWorld);

        Intersection bounceIntersection = scene.testIntersect(bounceRay);
        if (!bounceIntersection.hit) { break; }

        sample.eyePoints.push_back(bounceIntersection.point);

        const float invPDF = 1.f / bsdfSample.pdf;
        const float cosTheta = WorldFrame::absCosTheta(lastIntersection.shadingNormal, bsdfSample.wiWorld);

        modulation *= bsdfSample.throughput
            * cosTheta
            * invPDF;

        if (modulation.isBlack()) {
            break;
        }

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

    Color result(0.f);

    result += directSampleLights(
        intersection,
        bsdfSample,
        scene,
        random,
        sample
    );

    result += directSampleBSDF(
        intersection,
        bsdfSample,
        scene,
        random,
        sample
    );

    return result;
}

Color PathTracer::directSampleLights(
    const Intersection &intersection,
    const BSDFSample &bsdfSample,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    if (bsdfSample.material->isDelta()) { return 0.f; }

    const LightSample lightSample = scene.sampleDirectLights(intersection.point, random);

    const Vector3 lightDirection = (lightSample.point - intersection.point).toVector();
    const Vector3 wiWorld = lightDirection.normalized();

    if (lightSample.normal.dot(wiWorld) >= 0.f) {
        // Sample hit back of light
        sample.shadowTests.push_back({
            intersection.point,
            lightSample.point,
            true
        });

        return Color(0.f);
    }

    const Ray shadowRay = Ray(intersection.point, wiWorld);
    const float lightDistance = lightDirection.length();
    const bool occluded = scene.testOcclusion(shadowRay, lightDistance);

    sample.shadowTests.push_back({
        intersection.point,
        lightSample.point,
        occluded
    });

    if (occluded) {
        return Color(0.f);
    }

    const float pdf = lightSample.solidAnglePDF(intersection.point);
    const float brdfPDF = bsdfSample.material->pdf(intersection, wiWorld);
    const float lightWeight = MIS::balanceWeight(1, 1, pdf, brdfPDF);

    const Vector3 lightWo = -lightDirection.normalized();

    const Color lightContribution = lightSample.light->emit(lightWo)
        * lightWeight
        * intersection.material->f(intersection, wiWorld)
        * WorldFrame::absCosTheta(intersection.shadingNormal, wiWorld)
        / pdf;

    return lightContribution;
}

Color PathTracer::directSampleBSDF(
    const Intersection &intersection,
    const BSDFSample &bsdfSample,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    const Ray bounceRay(intersection.point, bsdfSample.wiWorld);
    const Intersection bounceIntersection = scene.testIntersect(bounceRay);

    if (bounceIntersection.hit && bounceIntersection.isEmitter()
        && bounceIntersection.woWorld.dot(bounceIntersection.shadingNormal) >= 0.f
    ) {
        const float distance = (bounceIntersection.point - intersection.point).toVector().length();
        const float lightPDF = scene.lightsPDF(
            intersection.point,
            bounceIntersection,
            Measure::SolidAngle
        );
        const float brdfWeight = bsdfSample.material->isDelta()
            ? 1.f
            : MIS::balanceWeight(1, 1, bsdfSample.pdf, lightPDF);

        const Color brdfContribution = bounceIntersection.material->emit()
            * brdfWeight
            * bsdfSample.throughput
            * WorldFrame::absCosTheta(intersection.shadingNormal, bsdfSample.wiWorld)
            / bsdfSample.pdf;

        return brdfContribution;
    } else if (!bounceIntersection.hit) {
        const Color environmentL = scene.environmentL(bsdfSample.wiWorld);
        if (!environmentL.isBlack()) {
            const float lightPDF = scene.environmentPDF(bsdfSample.wiWorld, Measure::SolidAngle);
            const float brdfWeight = bsdfSample.material->isDelta()
                ? 1.f
                : MIS::balanceWeight(1, 1, bsdfSample.pdf, lightPDF);

            const Color brdfContribution = environmentL
                * brdfWeight
                * bsdfSample.throughput
                * WorldFrame::absCosTheta(intersection.shadingNormal, bsdfSample.wiWorld)
                / bsdfSample.pdf;

            return brdfContribution;
        }
    }

    return Color(0.f);
}
