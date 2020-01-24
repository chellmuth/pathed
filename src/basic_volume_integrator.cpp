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

    Interaction fakeInteraction({ true, Point3(0.f, 0.f, 0.f ), Vector3(0.f), Vector3(0.f) });
    LoopState state({1, lastIntersection, modulation, mediumPtr, result, bsdfSample, fakeInteraction});

    for (int bounce = 2; !m_bounceController.checkDone(bounce); bounce++) {
        state.bounce = bounce;

        Ray bounceRay = state.interaction.isSurface
            ? Ray(state.lastIntersection.point, state.bsdfSample.wiWorld)
            : Ray(state.interaction.point, state.interaction.wiWorld)
        ;

        Intersection bounceIntersection = scene.testIntersect(bounceRay);
        if (!bounceIntersection.hit) { break; }

        if (state.interaction.isSurface) {
            const float invPDF = 1.f / state.bsdfSample.pdf;

            const Vector3 shadingNormal = state.lastIntersection.shadingNormal;
            const float cosTheta = WorldFrame::absCosTheta(shadingNormal, state.bsdfSample.wiWorld);

            state.modulation *= state.bsdfSample.throughput
                * cosTheta
                * invPDF;
        }

        if (state.modulation.isBlack()) { break; }

        state.mediumPtr = updateMediumPtr(state);

        IntegrationResult integrationResult = IntegrationHelper::noScatter();
        if (state.interaction.isSurface) {
            sample.eyePoints.push_back(bounceIntersection.point);

            integrationResult = scatter(
                state.mediumPtr,
                state.lastIntersection.point,
                bounceIntersection.point,
                scene,
                random
            );

        } else {
            integrationResult = scatter(
                state.mediumPtr,
                state.interaction.point,
                bounceIntersection.point,
                scene,
                random
            );
        }

        finishIt(
            state,
            integrationResult,
            bounceIntersection,
            scene,
            random,
            sample
       );
    }

    return state.result;
}

std::shared_ptr<Medium> BasicVolumeIntegrator::updateMediumPtr(LoopState &state) const {
    if (state.interaction.isSurface) {
        // Refraction, medium changes
        if (state.lastIntersection.woWorld.dot(state.bsdfSample.wiWorld) < 0.f) {
            // Internal, entering medium
            if (state.lastIntersection.normal.dot(state.bsdfSample.wiWorld) < 0.f) {
                return state.lastIntersection.surface->getInternalMedium();
            } else { // External, exiting medium
                return nullptr;
            }
        } // else Reflection, medium does not change
    }
    return state.mediumPtr;
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

void BasicVolumeIntegrator::finishIt(
    LoopState &state,
    const IntegrationResult &integrationResult,
    const Intersection &bounceIntersection,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    int bounce = state.bounce;
    Color &modulation = state.modulation;
    BSDFSample &bsdfSample = state.bsdfSample;
    std::shared_ptr<Medium> &mediumPtr = state.mediumPtr;
    Color &result = state.result;

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

        state.interaction = scatterInteraction;
        return;
    }

    bsdfSample = bounceIntersection.material->sample(
        bounceIntersection, random
    );

    state.lastIntersection = bounceIntersection;

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
}
