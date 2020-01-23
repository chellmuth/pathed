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

    Interaction fakeInteraction({ false, Point3(0.f, 0.f, 0.f ), Vector3(0.f)});
    LoopState state({1, lastIntersection, modulation, mediumPtr, result, bsdfSample, fakeInteraction});

    for (int bounce = 2; !m_bounceController.checkDone(bounce); bounce++) {
        state.bounce = bounce;

        if (!state.interaction.isSurface) {
            if (!processScatter(state, scene, random)) { break; }
        } else {
            if (!processBounce(state, scene, random, sample)) { break; }
        }
    }

    return state.result;
}

bool BasicVolumeIntegrator::processScatter(
    LoopState &state,
    const Scene &scene,
    RandomGenerator &random
) const {
    int bounce = state.bounce;
    Interaction &interaction = state.interaction;
    Color &modulation = state.modulation;
    std::shared_ptr<Medium> &mediumPtr = state.mediumPtr;
    Color &result = state.result;
    BSDFSample &bsdfSample = state.bsdfSample;

    const Phase phaseFunction;
    const Vector3 woWorld = interaction.woWorld;
    const Vector3 wiWorld = phaseFunction.sample(woWorld, random);
    const float sampleF = phaseFunction.f(woWorld, wiWorld);

    return true;
}

bool BasicVolumeIntegrator::processBounce(
    LoopState &state,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    int bounce = state.bounce;
    Intersection &lastIntersection = state.lastIntersection;
    Color &modulation = state.modulation;
    std::shared_ptr<Medium> &mediumPtr = state.mediumPtr;
    Color &result = state.result;
    BSDFSample &bsdfSample = state.bsdfSample;

    Ray bounceRay(lastIntersection.point, bsdfSample.wiWorld);

    const Point3 point = lastIntersection.point;
    const Vector3 woWorld = lastIntersection.woWorld;
    const Vector3 normal = lastIntersection.normal;
    const Vector3 shadingNormal = lastIntersection.shadingNormal;
    const Surface *surfacePtr = lastIntersection.surface;

    // Refraction, medium changes
    if (woWorld.dot(bsdfSample.wiWorld) < 0.f) {
        // Internal, entering medium
        if (normal.dot(bsdfSample.wiWorld) < 0.f) {
            mediumPtr = surfacePtr->getInternalMedium();
        } else { // External, exiting medium
            mediumPtr = nullptr;
        }
    } // else Reflection, medium does not change

    Intersection bounceIntersection = scene.testIntersect(bounceRay);
    if (!bounceIntersection.hit) { return false; }

    sample.eyePoints.push_back(bounceIntersection.point);

    const float invPDF = 1.f / bsdfSample.pdf;
    const float cosTheta = WorldFrame::absCosTheta(shadingNormal, bsdfSample.wiWorld);

    modulation *= bsdfSample.throughput
        * cosTheta
        * invPDF;

    if (modulation.isBlack()) {
        return false;
    }

    // if volume, integrate!
    const IntegrationResult integrationResult = scatter(
        mediumPtr,
        point,
        bounceIntersection.point,
        scene,
        random
    );
    if (integrationResult.shouldScatter) {
        const Color Ld = integrationResult.Ld;
        result += Ld * modulation;

        modulation *= integrationResult.transmittance;

        Interaction scatterInteraction({
            false,
            integrationResult.scatterPoint,
            -bounceRay.direction()
        });

        state.interaction = scatterInteraction;
        return true;
    } else {
        modulation *= transmittance(
            mediumPtr,
            point,
            bounceIntersection.point
        );
    }

    bsdfSample = bounceIntersection.material->sample(
        bounceIntersection, random
    );

    lastIntersection = bounceIntersection;

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

        sample.contributions.push_back({result - previous, invPDF});
    }

    return true;
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
