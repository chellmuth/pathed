#include "basic_volume_integrator.h"

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

#include <assert.h>
#include <fstream>
#include <iostream>

static Interaction surfaceInteraction() {
    return Interaction({
        true,
        Point3(0.f, 0.f, 0.f),
        Vector3(0.f),
        0.f,
        0.f,
        0.f,
        nullptr
    });
}

Color BasicVolumeIntegrator::L(
    const Point3 &point,
    const Vector3 &direction,
    const Scene &scene,
    RandomGenerator &random
) const {
    const Ray ray(point, direction);
    const Intersection intersection = scene.testIntersect(ray);

    if (!intersection.hit) { return Color(0.f); }

    const std::shared_ptr<Medium> medium = intersection.surface->getInternalMedium();

    if (medium) {
        const Interaction nextInteraction = sampleInteraction2(
            point,
            intersection,
            ray,
            scene,
            random
        );

        if (nextInteraction.isSurface) {
            return L(
                intersection.point,
                -intersection.woWorld,
                scene, random
            );
        } else {
            return directSampleLights(
                nextInteraction,
                scene,
                random
            );

            // Single scatter only

            // return L(
            //     nextInteraction.point,
            //     nextInteraction.direction,
            //     scene, random
            // );
        }
    } else {
        // Right now all emitters are in a vaccuum
        const Color emit = intersection.material->emit();
        if (emit.isBlack()) {
            return L(
                intersection.point,
                -intersection.woWorld,
                scene, random
            );
        } else {
            return emit;
        }
    }
}

Color BasicVolumeIntegrator::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    return L(intersection.point, -intersection.woWorld, scene, random);

    // if (!intersection.hit) { return Color(0.f); }

    // if (intersection.normal.dot(intersection.woWorld) < 0.f) {
    //     return Color(0.f, 0.f, 0.f);
    // }

    // Ray volumeRay(intersection.point, -intersection.woWorld);
    // Intersection exitIntersection = scene.testIntersect(volumeRay);

    // // Box edges
    // if (!exitIntersection.hit) { return Color(1.f, 0.f, 0.f); }

    // const Interaction nextInteraction = sampleInteraction(
    //     intersection,
    //     exitIntersection,
    //     volumeRay,
    //     random
    // );

    // if (nextInteraction.isSurface) {
    //     return Color(0.f, 0.f, 0.f);
    // } else {
    //     return Color(1.f, 1.f, 1.f);
    // }

    /////////////////////////////////////////////////////////////////////////////////////////

    // sample.eyePoints.push_back(intersection.point);

    // BSDFSample bsdfSample = intersection.material->sample(intersection, random);

    // Color result(0.f);
    // if (m_bounceController.checkCounts(1)) {
    //     result = direct(intersection, bsdfSample, scene, random, sample);
    //     sample.contributions.push_back({result, 1.f});
    // }

    // Color modulation = Color(1.f);
    // Intersection lastIntersection = intersection;

    // for (int bounce = 2; !m_bounceController.checkDone(bounce); bounce++) {
    //     Ray bounceRay(lastIntersection.point, bsdfSample.wiWorld);

    //     Intersection bounceIntersection = scene.testIntersect(bounceRay);
    //     if (!bounceIntersection.hit) { break; }

    //     sample.eyePoints.push_back(bounceIntersection.point);

    //     const Interaction nextInteraction = sampleInteraction(
    //         lastIntersection,
    //         bounceIntersection,
    //         bounceRay,
    //         random
    //     );

    //     if (nextInteraction.isSurface) {
    //         // ...
    //     } else {
    //         // modulation *= nextInteraction.sigmaS
    //         //     * (1.f / nextInteraction.sigmaT);

    //     }

    //     const float invPDF = 1.f / bsdfSample.pdf;
    //     const float cosTheta = WorldFrame::absCosTheta(lastIntersection.shadingNormal, bsdfSample.wiWorld);

    //     modulation *= bsdfSample.throughput
    //         * transmittance(
    //             lastIntersection,
    //             bounceIntersection
    //         )
    //         * cosTheta
    //         * invPDF;

    //     if (modulation.isBlack()) {
    //         break;
    //     }

    //     bsdfSample = bounceIntersection.material->sample(
    //         bounceIntersection, random
    //     );

    //     lastIntersection = bounceIntersection;

    //     if (m_bounceController.checkCounts(bounce)) {
    //         const Color previous = result;

    //         Color Ld = direct(bounceIntersection, bsdfSample, scene, random, sample);
    //         result += Ld * modulation;

    //         sample.contributions.push_back({result - previous, invPDF});
    //     }
    // }

    // return result;
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

Interaction BasicVolumeIntegrator::sampleInteraction(
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
        mediumOut->sigmaT(interactionPoint),
        nullptr
    });
}

Interaction BasicVolumeIntegrator::sampleInteraction2(
    const Point3 &sourcePoint,
    const Intersection &targetIntersection,
    const Ray &ray,
    const Scene &scene,
    RandomGenerator &random
) const {
    const std::shared_ptr<Medium> mediumOut = targetIntersection.surface->getInternalMedium();

    if (!mediumOut) { return surfaceInteraction(); }

    const TransmittanceQueryResult queryResult = mediumOut->findTransmittance(
        sourcePoint,
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
        mediumOut->sigmaT(interactionPoint),
        mediumOut.get()
    });
}

Color BasicVolumeIntegrator::directSampleLights(
    const Interaction &interaction,
    const Scene &scene,
    RandomGenerator &random
) const {
    const LightSample lightSample = scene.sampleDirectLights(interaction.point, random);

    const Vector3 lightDirection = (lightSample.point - interaction.point).toVector();
    const Vector3 wiWorld = lightDirection.normalized();

    if (lightSample.normal.dot(wiWorld) >= 0.f) {
        return Color(0.f);
    }

    const Ray shadowRay = Ray(interaction.point, wiWorld);
    const float lightDistance = lightDirection.length();
    const bool occluded = scene.testOcclusion(shadowRay, lightDistance);

    if (occluded) {
        return Color(0.f);
    } else {
        const float pdf = lightSample.solidAnglePDF(interaction.point);

        const Color transmittance = interaction.medium->transmittance(
            interaction.point,
            lightSample.point
        );

        return lightSample.light->emit(lightDirection)
            * transmittance
            * 1.f / (4.f * M_PI)
            / pdf;

    }
}

Color BasicVolumeIntegrator::transmittance(
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
