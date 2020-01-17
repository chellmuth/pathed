#include "volume_path_tracer.h"

#include "bounce_controller.h"
#include "color.h"
#include "direct_lighting_helper.h"
#include "monte_carlo.h"
#include "ray.h"
#include "transform.h"
#include "vector.h"

#include <iostream>


Color VolumePathTracer::L(
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

    for (int bounce = 2; !m_bounceController.checkDone(bounce); bounce++) {
        Ray bounceRay(lastIntersection.point, bsdfSample.wiWorld);

        // Refraction, medium changes
        if (lastIntersection.woWorld.dot(bsdfSample.wiWorld) < 0.f) {
            // Internal, entering medium
            if (lastIntersection.normal.dot(bsdfSample.wiWorld) < 0.f) {
                mediumPtr = lastIntersection.surface->getInternalMedium();
            } else { // External, exiting medium
                mediumPtr = nullptr;
            }
        } // else Reflection, medium does not change

        Intersection bounceIntersection = scene.testIntersect(bounceRay);
        if (!bounceIntersection.hit) { break; }

        sample.eyePoints.push_back(bounceIntersection.point);

        // if volume, integrate!
        const Color Ls = scatter(mediumPtr, lastIntersection, bounceIntersection, scene, random);
        result += Ls * modulation;

        const float invPDF = 1.f / bsdfSample.pdf;
        const float cosTheta = WorldFrame::absCosTheta(lastIntersection.shadingNormal, bsdfSample.wiWorld);

        modulation *= bsdfSample.throughput
            * transmittance(
                mediumPtr,
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
    }

    return result;
}

Color VolumePathTracer::transmittance(
    const std::shared_ptr<Medium> &mediumPtr,
    const Intersection &sourceIntersection,
    const Intersection &targetIntersection
) const {
    if (!mediumPtr) { return Color(1.f); }

    const Point3 &source = sourceIntersection.point;
    const Point3 &target = targetIntersection.point;

    return mediumPtr->transmittance(source, target);
}

Color VolumePathTracer::scatter(
    const std::shared_ptr<Medium> &mediumPtr,
    const Intersection &sourceIntersection,
    const Intersection &targetIntersection,
    const Scene &scene,
    RandomGenerator &random
) const {
    if (!mediumPtr) { return Color(0.f); }

    const Point3 &source = sourceIntersection.point;
    const Point3 &target = targetIntersection.point;

    return mediumPtr->integrate(source, target, scene, random);
}

