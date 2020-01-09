#include "volume_path_tracer.h"

#include "bounce_controller.h"
#include "color.h"
#include "globals.h"
#include "job.h"
#include "light.h"
#include "measure.h"
#include "mis.h"
#include "monte_carlo.h"
#include "phase.h"
#include "ray.h"
#include "transform.h"
#include "util.h"
#include "vector.h"

#include <fstream>
#include <iostream>

static Interaction surfaceInteraction() {
    return Interaction({
        true,
        Point3(0.f, 0.f, 0.f),
        Vector3(0.f),
        0.f,
        0.f,
        0.f
    });
}

Color VolumePathTracer::L(
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
        Ray bounceRay(lastIntersection.point, bsdfSample.wiWorld);

        Intersection bounceIntersection = scene.testIntersect(bounceRay);
        if (!bounceIntersection.hit) { break; }

        sample.eyePoints.push_back(bounceIntersection.point);

        const Interaction nextInteraction = sampleInteraction(
            lastIntersection,
            bounceIntersection,
            bounceRay,
            random
        );

        if (nextInteraction.isSurface) {
            // ...
        } else {
            // modulation *= nextInteraction.sigmaS
            //     * (1.f / nextInteraction.sigmaT);

        }

        const float invPDF = 1.f / bsdfSample.pdf;
        const float cosTheta = WorldFrame::absCosTheta(lastIntersection.shadingNormal, bsdfSample.wiWorld);

        modulation *= bsdfSample.throughput
            * transmittance(
                lastIntersection,
                bounceIntersection
            )
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

// static bool checkInternal(
//     const Intersection &sourceIntersection,
//     const Intersection &targetIntersection
// ) {
//     const Point3 &source = sourceIntersection.point;
//     const Point3 &target = targetIntersection.point;

//     const Vector3 sourceWi = (target - source).toVector().normalized();
//     const Vector3 targetWo = -sourceWi;

//     return (sourceWi.dot(sourceIntersection.normal) < 0.f)
//         && (targetWo.dot(targetIntersection.normal) < 0.f);
// }

Interaction VolumePathTracer::sampleInteraction(
    const Intersection &sourceIntersection,
    const Intersection &targetIntersection,
    const Ray &ray,
    RandomGenerator &random
) const {
    const std::shared_ptr<Medium> mediumIn = sourceIntersection.surface->getInternalMedium();
    const std::shared_ptr<Medium> mediumOut = targetIntersection.surface->getInternalMedium();

    if (!mediumIn || !mediumOut) { return surfaceInteraction(); }

    const TransmittanceQueryResult queryResult = mediumOut->findTransmittance(
        sourceIntersection.point,
        targetIntersection.point,
        random.next()
    );
    if (!queryResult.isValid) { return surfaceInteraction(); }

    const float distance = queryResult.distance;
    const Point3 interactionPoint = ray.at(distance);

    const Phase phaseFunction;
    const Vector3 woWorld = -ray.direction();
    const Vector3 wiWorld = phaseFunction.sample(woWorld, random);
    const float sampleF = phaseFunction.f(woWorld, wiWorld);

    return Interaction({
        false,
        interactionPoint,
        wiWorld,
        sampleF,
        mediumOut->sigmaS(interactionPoint),
        mediumOut->sigmaT(interactionPoint)
    });
}

Color VolumePathTracer::transmittance(
    const Intersection &sourceIntersection,
    const Intersection &targetIntersection
) const {
    // bool isInternal = checkInternal(sourceIntersection, targetIntersection);
    // if (!isInternal) { return Color(1.f); }

    const std::shared_ptr<Medium> mediumIn = sourceIntersection.surface->getInternalMedium();
    const std::shared_ptr<Medium> mediumOut = targetIntersection.surface->getInternalMedium();

    if (!mediumIn || !mediumOut) { return Color(1.f); }

    const Point3 &source = sourceIntersection.point;
    const Point3 &target = targetIntersection.point;

    return mediumOut->transmittance(source, target);
}

Color VolumePathTracer::direct(
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

Color VolumePathTracer::directSampleLights(
    const Intersection &intersection,
    const BSDFSample &bsdfSample,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    if (bsdfSample.material->isDelta()) { return 0.f; }

    const LightSample lightSample = scene.sampleDirectLights(intersection, random);

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

Color VolumePathTracer::directSampleBSDF(
    const Intersection &intersection,
    const BSDFSample &bsdfSample,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    const Ray bounceRay(intersection.point, bsdfSample.wiWorld);
    const Intersection bounceIntersection = scene.testIntersect(bounceRay);

    if (bounceIntersection.hit && bounceIntersection.isEmitter()) {
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
            const float lightPDF = scene.environmentPDF(bsdfSample.wiWorld);
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
