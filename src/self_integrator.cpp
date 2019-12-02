#include "self_integrator.h"

#include "camera.h"
#include "coordinate.h"
#include "globals.h"
#include "image.h"
#include "job.h"
#include "light.h"
#include "monte_carlo.h"
#include "path_tracer.h"
#include "photon_pdf.h"
#include "ray.h"
#include "transform.h"
#include "util.h"
#include "vector.h"

#include <iomanip>
#include <iostream>

static std::string zeroPad(int num, int fillCount)
{
    std::ostringstream stream;
    stream << std::setfill('0') << std::setw(fillCount) << num;
    return stream.str();
}

SelfIntegrator::SelfIntegrator(BounceController bounceController)
    : m_bounceController(bounceController), m_gtPDF(g_job->width(), g_job->height())
{
}

void SelfIntegrator::preprocess(const Scene &scene, RandomGenerator &random)
{
    Ray ray = scene.getCamera()->generateRay(184, 97);

    Intersection intersection = scene.testIntersect(ray);

    const int width = g_job->width();
    const int height = g_job->height();

    std::vector<float> radianceLookup(3 * width * height);
    for (int i = 0; i < 3 * width * height; i++) {
        radianceLookup[i] = 0.f;
    }

    const int spp = 64;
    for (int i = 0; i < spp; i++) {
        renderPDF(radianceLookup, scene, intersection);
    }

    const int phiSteps = width;
    const int thetaSteps = height;

    for (int thetaStep = 0; thetaStep < height; thetaStep++) {
        for (int phiStep = 0; phiStep < width; phiStep++) {
            int index = 3 * (thetaStep * phiSteps + phiStep);
            m_gtPDF.splatStepped(phiStep, thetaStep, radianceLookup[index] / spp);
        }
    }
    m_gtPDF.build();

    m_gtPDF.save();
}

void SelfIntegrator::renderPDF(
    std::vector<float> &radianceLookup,
    const Scene &scene,
    const Intersection &intersection
) const {
    const int phiSteps = g_job->width();
    const int cosThetaSteps = g_job->height();

    PathTracer pathTracer(g_job->bounceController());

    RandomGenerator random;
    int bounceCount = 2;

    Transform hemisphereToWorld = normalToWorldSpace(
        intersection.normal,
        intersection.wo
    );

    for (int phiStep = 0; phiStep < phiSteps; phiStep++) {
        for (int cosThetaStep = 0; cosThetaStep < cosThetaSteps; cosThetaStep++) {
            float phi = m_gtPDF.phiAtStep(phiStep);
            float theta = m_gtPDF.thetaAtStep(cosThetaStep);

            float y = cosf(theta);
            float x = sinf(theta) * cosf(phi);
            float z = sinf(theta) * sinf(phi);

            Vector3 wiHemisphere(x, y, z);
            Vector3 wiWorld = hemisphereToWorld.apply(wiHemisphere);

            Ray ray = Ray(intersection.point, wiWorld);
            const Intersection fisheyeIntersection = scene.testIntersect(ray);
            Color sampleL(0.f);
            if (fisheyeIntersection.hit) {
                Sample sample;

                sampleL = pathTracer.L(
                    fisheyeIntersection,
                    scene,
                    random,
                    sample
                );
            }

            const float average = sampleL.average();
            radianceLookup[3 * (cosThetaStep * phiSteps + phiStep) + 0] += average;
            radianceLookup[3 * (cosThetaStep * phiSteps + phiStep) + 1] += average;
            radianceLookup[3 * (cosThetaStep * phiSteps + phiStep) + 2] += average;
        }
    }
}

Vector3 SelfIntegrator::nextBounce(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    float *pdf
) const
{
    float phi, theta;
    m_gtPDF.sample(random, &phi, &theta, pdf);

    // std::cout << phi << " " << theta << " " << *pdf << std::endl;

    Vector3 transformed = sphericalToCartesian(phi, theta);
    // transformed.debug();
    return transformed;
}

Color SelfIntegrator::L(
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
            lastIntersection.wo
        );

        float pdf;
        Vector3 bounceDirection = hemisphereToWorld.apply(nextBounce(intersection, scene, random, &pdf));

        // Vector3 hemisphereSample = CosineSampleHemisphere(random);
        // float pdf = CosineHemispherePdf(hemisphereSample);
        // Vector3 bounceDirection = hemisphereToWorld.apply(hemisphereSample);

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

Color SelfIntegrator::direct(
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
    SurfaceSample lightSample = light->sample(intersection, random);

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
