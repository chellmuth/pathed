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
    const float distance = direction.length();
    const Vector3 wi = direction.normalized();

    const Ray shadowRay(source, wi);

    bool occluded = scene.testOcclusion(shadowRay, distance);
    if (occluded) { return; }

    Pixel *pixel = nullptr;
    camera.calculatePixel(source, pixel);
    if (!pixel) { return; }

    const size_t index = 3 * (pixel->y * camera.getResolution().x + pixel->x);

    const Color brdf = intersection.material->f(intersection, wi);
    const float cosTheta = std::max(0.f, intersection.normal.dot(wi));

    const Color splattedRadiance = radiance * brdf * cosTheta / (distance * distance);

    lock.lock();
    radianceLookup[index + 0] += splattedRadiance.r();
    radianceLookup[index + 1] += splattedRadiance.g();
    radianceLookup[index + 2] += splattedRadiance.b();
    lock.unlock();
}

void LightTracer::measure(
    std::vector<float> &radianceLookup,
    const Scene &scene,
    RandomGenerator &random
) const {
    const LightSample lightSample = scene.sampleLights(random);

    const float sampleInvPDF = lightSample.invPDF * scene.lights().size();

    Color throughput = lightSample.light->emit() * sampleInvPDF;
    // splat(throughput, lightSample.point, scene, radianceLookup);

    const Vector3 hemisphereSample = UniformSampleHemisphere(random);
    const Transform hemisphereToWorld = normalToWorldSpace(lightSample.normal);

    const Vector3 wi = hemisphereToWorld.apply(hemisphereSample);

    throughput *= std::max(0.f, lightSample.normal.dot(wi));

    const float invPDF = M_TWO_PI;
    throughput *= invPDF;

    Ray lightRay(lightSample.point, wi);

    const int photonBounces = g_job->photonBounces();
    for (int bounce = 0; bounce < 2; bounce++) {
        Intersection intersection = scene.testIntersect(lightRay);
        if (!intersection.hit) { break; }

        splat(throughput, intersection, scene, radianceLookup);

        const Vector3 hemisphereSample = UniformSampleHemisphere(random);
        const Transform hemisphereToWorld = normalToWorldSpace(intersection.normal);
        const Vector3 wi = hemisphereToWorld.apply(hemisphereSample);
        lightRay = Ray(intersection.point, wi);

        throughput *= intersection.material->f(intersection, wi);
        throughput *= std::max(0.f, intersection.normal.dot(wi));

        const float invPDF = M_TWO_PI;
        throughput *= invPDF;

        if (throughput.isBlack()) { break; }
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
