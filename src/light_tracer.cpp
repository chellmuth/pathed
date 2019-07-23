#include "light_tracer.h"

#include "color.h"
#include "globals.h"
#include "job.h"
#include "light.h"
#include "monte_carlo.h"
#include "ray.h"
#include "transform.h"
#include "util.h"
#include "vector.h"

#include <assert.h>
#include <limits>
#include <math.h>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

static std::mutex lock;

static int maxBounces = 6;

void LightTracer::splat(
    const Color &radiance,
    const Intersection &intersection,
    const Scene &scene,
    std::vector<float> &radianceLookup
) const {
    const Point3 &source = intersection.point;
    const Camera &camera = *scene.getCamera();

    const Point3 origin = camera.getOrigin();
    const Vector3 direction = (origin - source).toVector();
    const Ray shadowRay(source, direction.normalized());

    bool occluded = scene.testOcclusion(shadowRay, direction.length());
    if (occluded) { return; }

    const std::optional<Pixel> wrappedPixel = camera.calculatePixel(source);
    if (!wrappedPixel) { return; }

    const Pixel pixel = wrappedPixel.value();
    const size_t index = 3 * (pixel.y * camera.getResolution().x + pixel.x);

    const float cosTheta = std::max(0.f, intersection.normal.dot(direction));
    const Color biradiance = radiance * cosTheta / (direction.length() * direction.length());

    lock.lock();
    radianceLookup[index + 0] += biradiance.r();
    radianceLookup[index + 1] += biradiance.g();
    radianceLookup[index + 2] += biradiance.b();
    lock.unlock();
}

void LightTracer::measure(
    std::vector<float> &radianceLookup,
    const Scene &scene,
    RandomGenerator &random
) const {
    const LightSample lightSample = scene.sampleLights(random);

    const float sampleInvPDF = lightSample.invPDF * scene.lights().size();

    Color throughput = lightSample.light->getMaterial()->emit() * sampleInvPDF;
    // splat(throughput, lightSample.point, scene, radianceLookup);

    const Vector3 hemisphereSample = UniformSampleHemisphere(random);
    const Transform hemisphereToWorld = normalToWorldSpace(lightSample.normal);

    Vector3 bounceDirection = hemisphereToWorld.apply(hemisphereSample);

    throughput *= std::max(0.f, lightSample.normal.dot(bounceDirection));

    Ray lightRay(lightSample.point, bounceDirection);

    const int photonBounces = g_job->photonBounces();
    for (int bounce = 0; bounce < photonBounces; bounce++) {
        Intersection intersection = scene.testIntersect(lightRay);
        if (!intersection.hit) { break; }

        // throughput *= std::max(0.f, intersection.wi.dot(intersection.normal * -1.f));
        throughput *= intersection.material->f(intersection, bounceDirection);

        const float invPDF = M_TWO_PI;
        throughput *= invPDF;

        if (throughput.isBlack()) { break; }

        splat(throughput, intersection, scene, radianceLookup);
        return;

        Vector3 hemisphereSample = UniformSampleHemisphere(random);
        Transform hemisphereToWorld = normalToWorldSpace(intersection.normal);
        bounceDirection = hemisphereToWorld.apply(hemisphereSample);
        lightRay = Ray(intersection.point, bounceDirection);

        // float invPDF = 1.f / INV_TWO_PI;
        // throughput *= invPDF;
    }
}

void LightTracer::sampleImage(
    std::vector<float> &radianceLookup,
    std::vector<Sample> &sampleLookup,
    Scene &scene,
    RandomGenerator &random
) {
    const int width = g_job->width();
    const int height = g_job->height();

    #pragma omp parallel for
    for (int i = 0; i < width * height; i++) {
        measure(
            radianceLookup,
            scene,
            random
        );
    }
}
