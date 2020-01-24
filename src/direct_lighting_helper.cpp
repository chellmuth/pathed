#include "direct_lighting_helper.h"

#include "intersection.h"
#include "material.h"
#include "measure.h"
#include "medium.h"
#include "mis.h"
#include "random_generator.h"
#include "ray.h"
#include "sample.h"
#include "scene.h"
#include "world_frame.h"
#include "vector.h"

#include <assert.h>
#include <vector>

static Color directSampleLights(
    const Intersection &intersection,
    const std::shared_ptr<Medium> &mediumPtr,
    const BSDFSample &bsdfSample,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
);

static Color directSampleBSDF(
    const Intersection &intersection,
    const std::shared_ptr<Medium> &mediumPtr,
    const BSDFSample &bsdfSample,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
);

Color DirectLightingHelper::Ld(
    const Intersection &intersection,
    const std::shared_ptr<Medium> &mediumPtr,
    const BSDFSample &bsdfSample,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) {
    if (bsdfSample.material->isContainer()) { return Color(0.f); }

    Color emit = intersection.material->emit();
    if (!emit.isBlack()) {
        // part of my old logic - if you hit an emitter, don't do direct lighting?
        return Color(0.f, 0.f, 0.f);
    }

    Color result(0.f);

    result += directSampleLights(
        intersection,
        mediumPtr,
        bsdfSample,
        scene,
        random,
        sample
    );

    result += directSampleBSDF(
        intersection,
        mediumPtr,
        bsdfSample,
        scene,
        random,
        sample
    );

    return result;
}

static Color directSampleLights(
    const Intersection &intersection,
    const std::shared_ptr<Medium> &mediumPtr,
    const BSDFSample &bsdfSample,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) {
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
    OcclusionResult occlusionResult = scene.testVolumetricOcclusion(shadowRay, lightDistance);

    sample.shadowTests.push_back({
        intersection.point,
        lightSample.point,
        occlusionResult.isOccluded
    });

    if (occlusionResult.isOccluded) {
        return Color(0.f);
    }

    Color transmittance(1.f);

    const std::vector<VolumeEvent> &volumeEvents = occlusionResult.volumeEvents;
    const size_t eventCount = volumeEvents.size();
    if (eventCount > 0) {
        if (mediumPtr) {
            assert(eventCount == 1);

            const Point3 enterPoint = intersection.point;
            const Point3 exitPoint = shadowRay.at(volumeEvents[0].t);

            transmittance *= mediumPtr->transmittance(enterPoint, exitPoint);
        } else {
            assert(eventCount == 2);

            const std::shared_ptr<Medium> mediumPtr = volumeEvents[0].mediumPtr;

            const Point3 enterPoint = shadowRay.at(volumeEvents[0].t);
            const Point3 exitPoint = shadowRay.at(volumeEvents[1].t);

            transmittance *= mediumPtr->transmittance(enterPoint, exitPoint);
        }
    }

    const float pdf = lightSample.solidAnglePDF(intersection.point);
    const float brdfPDF = bsdfSample.material->pdf(intersection, wiWorld);
    const float lightWeight = MIS::balanceWeight(1, 1, pdf, brdfPDF);

    const Vector3 lightWo = -lightDirection.normalized();

    const Color lightContribution = lightSample.light->emit(lightWo)
        * transmittance
        * lightWeight
        * intersection.material->f(intersection, wiWorld)
        * WorldFrame::absCosTheta(intersection.shadingNormal, wiWorld)
        / pdf;

    return lightContribution;
}

static Color directSampleBSDF(
    const Intersection &intersection,
    const std::shared_ptr<Medium> &mediumPtr,
    const BSDFSample &bsdfSample,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) {
    const Ray bounceRay(intersection.point, bsdfSample.wiWorld);
    const Intersection bounceIntersection = scene.testVolumetricIntersect(bounceRay);

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
