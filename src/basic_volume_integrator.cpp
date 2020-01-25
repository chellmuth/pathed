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


using MediumPtrVector = std::vector<std::shared_ptr<Medium> >;

static std::shared_ptr<Medium> currentMediumPtr(const MediumPtrVector &mediumPtrs) {
    const int size = mediumPtrs.size();
    if (size > 0) {
        return mediumPtrs[size - 1];
    }
    return nullptr;
}

Color BasicVolumeIntegrator::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    Color result(0.f);
    Color modulation = Color(1.f);
    MediumPtrVector mediumPtrs;

    sample.eyePoints.push_back(intersection.point);

    const BSDFSample bsdfSample = intersection.material->sample(intersection, random);
    Interaction interaction({
        true,
        intersection,
        bsdfSample,
        InteractionHelper::nullScatter()
    });

    if (m_bounceController.checkCounts(1)) {
        result = DirectLightingHelper::Ld(
            intersection,
            currentMediumPtr(mediumPtrs),
            bsdfSample,
            scene,
            random,
            sample
        );
        sample.contributions.push_back({result, 1.f});
    }

    for (int bounce = 2; !m_bounceController.checkDone(bounce); bounce++) {
        // We have already calculated direct light on interaction.point() with NEE

        // Intersect the next ray
        const Ray bounceRay(interaction.point(), interaction.wiWorld());

        const Intersection bounceIntersection = scene.testIntersect(bounceRay);
        if (!bounceIntersection.hit) { break; }

        // If we came from a surface, update the modulation
        if (interaction.isSurface) {
            const float invPDF = 1.f / interaction.bsdfSample.pdf;

            const Vector3 shadingNormal = interaction.intersection.shadingNormal;
            const float cosTheta = WorldFrame::absCosTheta(shadingNormal, interaction.bsdfSample.wiWorld);

            modulation *= interaction.bsdfSample.throughput
                * cosTheta
                * invPDF;
        }

        if (modulation.isBlack()) { break; }

        // Update the current medium
        updateMediumPtrs(mediumPtrs, interaction);

        sample.eyePoints.push_back(bounceIntersection.point);

        // Integrate any volumes,
        // possibly overriding the bounce point with a scatter point
        const IntegrationResult integrationResult = scatter(
            currentMediumPtr(mediumPtrs),
            interaction.point(),
            bounceIntersection.point,
            scene,
            random
        );

        if (integrationResult.shouldScatter) {
            modulation *= integrationResult.weight;

            // Direct light on the scatter point
            const Color Ld = integrationResult.Ld;
            result += Ld * modulation;

            const Phase phaseFunction;
            const Vector3 woWorld = bounceIntersection.woWorld;
            const Vector3 wiWorld = phaseFunction.sample(woWorld, random);

            ScatterEvent scatterEvent({
                integrationResult.scatterPoint,
                woWorld,
                wiWorld
            });

            interaction.isSurface = false;
            interaction.scatterEvent = scatterEvent;
        } else {
            // Grab the future bsdf ray to use in MIS direct light sampling
            interaction.bsdfSample = bounceIntersection.material->sample(
                bounceIntersection, random
            );

            if (m_bounceController.checkCounts(bounce)) {
                const Color previous = result;

                // Direct light on the bounce point
                Color Ld = DirectLightingHelper::Ld(
                    bounceIntersection,
                    currentMediumPtr(mediumPtrs),
                    interaction.bsdfSample,
                    scene,
                    random,
                    sample
                );
                result += Ld * modulation;
                // sample.contributions.push_back({result - previous, invPDF});
            }

            interaction.isSurface = true;
            interaction.intersection = bounceIntersection;
        }
    }

    return result;
}

void BasicVolumeIntegrator::updateMediumPtrs(
    std::vector<std::shared_ptr<Medium> > &mediumPtrs,
    const Interaction &interaction
) const {
    const Intersection &intersection = interaction.intersection;
    const BSDFSample &bsdfSample = interaction.bsdfSample;

    if (interaction.isSurface) {
        // Refraction, medium changes
        if (intersection.woWorld.dot(bsdfSample.wiWorld) < 0.f) {
            // Internal, entering medium
            if (intersection.normal.dot(bsdfSample.wiWorld) < 0.f) {
                auto mediumPtr = intersection.surface->getInternalMedium();
                mediumPtrs.push_back(mediumPtr);
                return;
            } else { // External, exiting medium
                // todo: shouldn't need this!
                if (mediumPtrs.size() > 0) {
                    mediumPtrs.pop_back();
                }
                return;
            }
        } // else Reflection, medium does not change
    }
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
