#include "volume_helper.h"

#include "medium.h"
#include "point.h"
#include "random_generator.h"
#include "ray.h"
#include "vector.h"
#include "scene.h"

#include <assert.h>

Color VolumeHelper::directSampleLights(
    const Medium &medium,
    const Point3 &samplePoint,
    const Scene &scene,
    RandomGenerator &random
) {
    const LightSample lightSample = scene.sampleDirectLights(samplePoint, random);

    const Vector3 sampleDirection = (lightSample.point - samplePoint).toVector();
    const Vector3 wiWorld = sampleDirection.normalized();

    if (lightSample.normal.dot(wiWorld) >= 0.f) {
        return Color(0.f);
    }

    const Ray shadowRay = Ray(samplePoint, wiWorld);
    const float lightDistance = sampleDirection.length();
    const OcclusionResult occlusionResult = scene.testVolumetricOcclusion(shadowRay, lightDistance);

    if (occlusionResult.isOccluded) {
        return Color(0.f);
    } else {
        const float pdf = lightSample.solidAnglePDF(samplePoint);
        const Vector3 lightWo = -sampleDirection.normalized();

        // TODO: Over all intersected volumes
        Color shadowTransmittance(0.f);

        const std::vector<VolumeEvent> &volumeEvents = occlusionResult.volumeEvents;
        const size_t eventCount = volumeEvents.size();
        if (eventCount > 0) {
            assert(eventCount == 1);

            const Point3 enterPoint = samplePoint;
            const Point3 exitPoint = shadowRay.at(volumeEvents[0].t);

            shadowTransmittance = medium.transmittance(enterPoint, exitPoint);
        }

        return lightSample.light->emit(lightWo)
            * shadowTransmittance
            * 1.f / (4.f * M_PI)
            / pdf;
    }
}

Color VolumeHelper::rayTransmission(
    const Ray &ray,
    const std::vector<VolumeEvent> &volumeEvents,
    const std::shared_ptr<Medium> &mediumPtr
) {
    Color transmittance(1.f);

    const size_t eventCount = volumeEvents.size();
    if (eventCount > 0) {
        if (mediumPtr) {
            assert(eventCount == 1 || eventCount == 2);

            if (eventCount == 1) {
                const Point3 enterPoint = ray.origin();
                const Point3 exitPoint = ray.at(volumeEvents[0].t);

                transmittance *= mediumPtr->transmittance(enterPoint, exitPoint);
            } else if (eventCount == 2) {
                // TODO: Remove tnear in embree, and remove this code!
                // If there are two events, assume we leaked out of the volume
                // without intersecting the passthrough model.
                //
                // This fixes the direct lighting, but our integrator will still
                // believe it is inside the volume, affecting future bounces
                const Point3 enterPoint = ray.at(volumeEvents[0].t);
                const Point3 exitPoint = ray.at(volumeEvents[1].t);

                transmittance *= mediumPtr->transmittance(enterPoint, exitPoint);
            }
        } else {
            assert(eventCount == 1 || eventCount == 2);

            const std::shared_ptr<Medium> mediumPtr = volumeEvents[0].mediumPtr;

            if (eventCount == 2) {
                const Point3 enterPoint = ray.at(volumeEvents[0].t);
                const Point3 exitPoint = ray.at(volumeEvents[1].t);

                transmittance *= mediumPtr->transmittance(enterPoint, exitPoint);
            } else if (eventCount == 1) {
                // TODO: Remove tnear in embree, and remove this code!
                // If there's only one event, assume the volume was too close to
                // the point to register as a hit
                const Point3 enterPoint = ray.origin();
                const Point3 exitPoint = ray.at(volumeEvents[0].t);

                transmittance *= mediumPtr->transmittance(enterPoint, exitPoint);
            }
        }
    }

    return transmittance;
}
