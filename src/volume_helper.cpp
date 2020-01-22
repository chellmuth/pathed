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

    const Vector3 lightDirection = (lightSample.point - samplePoint).toVector();
    const Vector3 wiWorld = lightDirection.normalized();

    if (lightSample.normal.dot(wiWorld) >= 0.f) {
        return Color(0.f);
    }

    const Ray shadowRay = Ray(samplePoint, wiWorld);
    const float lightDistance = lightDirection.length();
    const OcclusionResult occlusionResult = scene.testVolumetricOcclusion(shadowRay, lightDistance);

    if (occlusionResult.isOccluded) {
        return Color(0.f);
    } else {
        const float pdf = lightSample.solidAnglePDF(samplePoint);

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

        return lightSample.light->emit(-lightDirection.normalized())
            * shadowTransmittance
            * 1.f / (4.f * M_PI)
            / pdf;
    }
}
