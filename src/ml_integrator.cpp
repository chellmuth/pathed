#include "ml_integrator.h"

#include "globals.h"
#include "job.h"
#include "light.h"
#include "monte_carlo.h"
#include "ray.h"
#include "transform.h"
#include "util.h"
#include "vector.h"

#include <iostream>

MLIntegrator::MLIntegrator(BounceController bounceController)
    : m_bounceController(bounceController)
{
    m_dataSource = std::make_shared<DataSource>();
}

Vector3 MLIntegrator::sample(const Vector3 &normal, RandomGenerator &random, float *pdf)
{
    Vector3 hemisphereSample = UniformSampleHemisphere(random);
    Transform hemisphereToWorld = normalToWorldSpace(normal);
    *pdf = INV_TWO_PI;
    return hemisphereToWorld.apply(hemisphereSample);
}

void MLIntegrator::createPhotons(const Scene &scene, RandomGenerator &random)
{
    const int photonSamples = g_job->photonSamples();
    const int photonBounces = g_job->photonBounces();

    for (int i = 0; i < photonSamples; i++) {
        LightSample lightSample = scene.sampleLights(random);

        float pdf;
        Vector3 bounceDirection = sample(lightSample.normal, random, &pdf);

        if (bounceDirection.dot(lightSample.normal) < 0.f) {
            assert(false);
            break;
        }

        Ray lightRay(lightSample.point, bounceDirection);

        Color throughput = lightSample.light->getMaterial()->emit();
        for (int bounce = 0; bounce < photonBounces; bounce++) {
            Intersection intersection = scene.testIntersect(lightRay);
            if (!intersection.hit) { break; }

            throughput *= fmaxf(0.f, intersection.wi.dot(intersection.normal * -1.f));
            if (throughput.isBlack()) { break; }

            if (bounce > 0) { // don't guide towards direct lights
                m_dataSource->points.push_back({
                    intersection.point.x(),
                    intersection.point.y(),
                    intersection.point.z(),
                    lightRay.origin(),
                    throughput
                });
            }

            Vector3 hemisphereSample = UniformSampleHemisphere(random);
            Transform hemisphereToWorld = normalToWorldSpace(intersection.normal);
            bounceDirection = hemisphereToWorld.apply(hemisphereSample);
            lightRay = Ray(intersection.point, bounceDirection);

            throughput *= intersection.material->f(intersection, bounceDirection);

            // float invPDF = 1.f / INV_TWO_PI;
            // throughput *= invPDF;
        }
    }
}

void MLIntegrator::preprocess(const Scene &scene, RandomGenerator &random)
{
    createPhotons(scene, random);

    m_KDTree = std::make_unique<KDTree>(3, *m_dataSource, nanoflann::KDTreeSingleIndexAdaptorParams(10));
    m_KDTree->buildIndex();
}

Color MLIntegrator::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    Color result(0.f);
    if (m_bounceController.checkCounts(1)) {
        result = direct(intersection, scene, random);
    }

    Color modulation = Color(1.f);
    Intersection lastIntersection = intersection;

    for (int bounce = 2; !m_bounceController.checkDone(bounce); bounce++) {
        Transform hemisphereToWorld = normalToWorldSpace(
            lastIntersection.normal,
            lastIntersection.wi
        );

        Vector3 hemisphereSample = CosineSampleHemisphere(random);
        float pdf = CosineHemispherePdf(hemisphereSample);

        Vector3 bounceDirection = hemisphereToWorld.apply(hemisphereSample);
        Ray bounceRay(
            lastIntersection.point,
            bounceDirection
        );

        Intersection bounceIntersection = scene.testIntersect(bounceRay);
        if (!bounceIntersection.hit) { break; }

        Color f = lastIntersection.material->f(lastIntersection, bounceDirection);
        float invPDF = 1.f / pdf;

        modulation *= f
            * fmaxf(0.f, bounceDirection.dot(lastIntersection.normal))
            * invPDF;
        lastIntersection = bounceIntersection;

        if (m_bounceController.checkCounts(bounce)) {
            const Color previous = result;

            result += direct(bounceIntersection, scene, random) * modulation;
        }
    }

    return result;
}

Color MLIntegrator::direct(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random
) const {
    Color emit = intersection.material->emit();
    if (!emit.isBlack()) {
        // part of my old logic - if you hit an emitter, don't do direct lighting?
        return Color(0.f, 0.f, 0.f);
    }

    int lightCount = scene.lights().size();
    int lightIndex = (int)floorf(random.next() * lightCount);

    std::shared_ptr<Light> light = scene.lights()[lightIndex];
    SurfaceSample lightSample = light->sample(random);

    Vector3 lightDirection = (lightSample.point - intersection.point).toVector();
    Vector3 wo = lightDirection.normalized();

    if (lightSample.normal.dot(wo) >= 0.f) {
        return Color(0.f);
    }

    Ray shadowRay = Ray(intersection.point, wo);
    float lightDistance = lightDirection.length();
    bool occluded = scene.testOcclusion(shadowRay, lightDistance);

    if (occluded) {
        return Color(0.f);
    }

    float invPDF = lightSample.invPDF * lightCount;

    return light->biradiance(lightSample, intersection.point)
        * intersection.material->f(intersection, wo)
        * fmaxf(0.f, wo.dot(intersection.normal))
        * invPDF;
}
