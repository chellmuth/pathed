#include "volume_path_tracer.h"

#include "bounce_controller.h"
#include "color.h"
#include "direct_lighting_helper.h"
#include "measure.h"
#include "monte_carlo.h"
#include "phase.h"
#include "ray.h"
#include "transform.h"
#include "util.h"
#include "vector.h"

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
        result = DirectLightingHelper::Ld(intersection, bsdfSample, scene, random, sample);
        sample.contributions.push_back({result, 1.f});
    }

    Color modulation = Color(1.f);
    Intersection lastIntersection = intersection;

    for (int bounce = 2; !m_bounceController.checkDone(bounce); bounce++) {
        Ray bounceRay(lastIntersection.point, bsdfSample.wiWorld);

        Intersection bounceIntersection = scene.testIntersect(bounceRay);
        if (!bounceIntersection.hit) { break; }

        sample.eyePoints.push_back(bounceIntersection.point);

        // if volume, integrate!
        const Color Ls = scatter(lastIntersection, bounceIntersection, scene, random);
        result += Ls * modulation;

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

            Color Ld = DirectLightingHelper::Ld(bounceIntersection, bsdfSample, scene, random, sample);
            result += Ld * modulation;

            sample.contributions.push_back({result - previous, invPDF});
        }
    }

    return result;
}

Color VolumePathTracer::transmittance(
    const Intersection &sourceIntersection,
    const Intersection &targetIntersection
) const {
    const std::shared_ptr<Medium> mediumIn = sourceIntersection.surface->getInternalMedium();
    const std::shared_ptr<Medium> mediumOut = targetIntersection.surface->getInternalMedium();

    if (!mediumIn || !mediumOut) { return Color(1.f); }

    const Point3 &source = sourceIntersection.point;
    const Point3 &target = targetIntersection.point;

    return mediumOut->transmittance(source, target);
}

Color VolumePathTracer::scatter(
    const Intersection &sourceIntersection,
    const Intersection &targetIntersection,
    const Scene &scene,
    RandomGenerator &random
) const {
    const std::shared_ptr<Medium> mediumIn = sourceIntersection.surface->getInternalMedium();
    const std::shared_ptr<Medium> mediumOut = targetIntersection.surface->getInternalMedium();

    if (!mediumIn || !mediumOut) { return Color(0.f); }

    const Point3 &source = sourceIntersection.point;
    const Point3 &target = targetIntersection.point;

    return mediumOut->integrate(source, target, scene, random);
}

