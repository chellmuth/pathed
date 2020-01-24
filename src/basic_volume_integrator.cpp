#include "basic_volume_integrator.h"

#include "bounce_controller.h"
#include "color.h"
#include "direct_lighting_helper.h"
#include "monte_carlo.h"
#include "phase.h"
#include "ray.h"
#include "transform.h"
#include "vector.h"

#include <iostream>


Color BasicVolumeIntegrator::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    std::shared_ptr<Medium> mediumPtr(nullptr);

    sample.eyePoints.push_back(intersection.point);

    BSDFSample bsdfSample = intersection.material->sample(intersection, random);

    Color result(0.f);
    if (m_bounceController.checkCounts(1)) {
        result = DirectLightingHelper::Ld(intersection, mediumPtr, bsdfSample, scene, random, sample);
        sample.contributions.push_back({result, 1.f});
    }

    Color modulation = Color(1.f);
    Intersection lastIntersection = intersection;

    Interaction interaction({ true, Point3(0.f, 0.f, 0.f ), Vector3(0.f), Vector3(0.f) });

    for (int bounce = 2; !m_bounceController.checkDone(bounce); bounce++) {
        const Point3 lastPoint = interaction.isSurface
            ? lastIntersection.point
            : interaction.point
        ;

        const Vector3 lastDirection = interaction.isSurface
            ? bsdfSample.wiWorld
            : interaction.wiWorld
        ;

        const Ray bounceRay(lastPoint, lastDirection);

        const Intersection bounceIntersection = scene.testIntersect(bounceRay);
        if (!bounceIntersection.hit) { break; }

        if (interaction.isSurface) {
            const float invPDF = 1.f / bsdfSample.pdf;

            const Vector3 shadingNormal = lastIntersection.shadingNormal;
            const float cosTheta = WorldFrame::absCosTheta(shadingNormal, bsdfSample.wiWorld);

            modulation *= bsdfSample.throughput
                * cosTheta
                * invPDF;
        }

        if (modulation.isBlack()) { break; }

        mediumPtr = updateMediumPtr(
            mediumPtr,
            interaction,
            lastIntersection,
            bsdfSample
        );

        sample.eyePoints.push_back(bounceIntersection.point);

        const IntegrationResult integrationResult = scatter(
            mediumPtr,
            lastPoint,
            bounceIntersection.point,
            scene,
            random
        );

        if (integrationResult.shouldScatter) {
            modulation *= integrationResult.weight;

            const Color Ld = integrationResult.Ld;
            result += Ld * modulation;

            const Phase phaseFunction;
            const Vector3 woWorld = bounceIntersection.woWorld;
            const Vector3 wiWorld = phaseFunction.sample(woWorld, random);

            Interaction scatterInteraction({
                false,
                integrationResult.scatterPoint,
                woWorld,
                wiWorld
            });

            interaction = scatterInteraction;
        } else {
            bsdfSample = bounceIntersection.material->sample(
                bounceIntersection, random
            );

            if (m_bounceController.checkCounts(bounce)) {
                const Color previous = result;

                Color Ld = DirectLightingHelper::Ld(
                    bounceIntersection,
                    mediumPtr,
                    bsdfSample,
                    scene,
                    random,
                    sample
                );
                result += Ld * modulation;
                // sample.contributions.push_back({result - previous, invPDF});
            }
            lastIntersection = bounceIntersection;
        }
    }

    return result;
}

std::shared_ptr<Medium> BasicVolumeIntegrator::updateMediumPtr(
    const std::shared_ptr<Medium> mediumPtr,
    const Interaction &interaction,
    const Intersection &lastIntersection,
    const BSDFSample &bsdfSample
) const {
    if (interaction.isSurface) {
        // Refraction, medium changes
        if (lastIntersection.woWorld.dot(bsdfSample.wiWorld) < 0.f) {
            // Internal, entering medium
            if (lastIntersection.normal.dot(bsdfSample.wiWorld) < 0.f) {
                return lastIntersection.surface->getInternalMedium();
            } else { // External, exiting medium
                return nullptr;
            }
        } // else Reflection, medium does not change
    }
    return mediumPtr;
}

Color BasicVolumeIntegrator::transmittance(
    const std::shared_ptr<Medium> &mediumPtr,
    const Point3 &source,
    const Point3 &target
) const {
    if (!mediumPtr) { return Color(1.f); }

    return mediumPtr->transmittance(source, target);
}

IntegrationResult BasicVolumeIntegrator::scatter(
    const std::shared_ptr<Medium> &mediumPtr,
    const Point3 &source,
    const Point3 &target,
    const Scene &scene,
    RandomGenerator &random
) const {
    if (!mediumPtr) { return IntegrationHelper::noScatter(); }

    const IntegrationResult result = mediumPtr->integrate(source, target, scene, random);
    return result;
}
