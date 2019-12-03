#include "path_tracer.h"

#include "bounce_controller.h"
#include "color.h"
#include "globals.h"
#include "job.h"
#include "light.h"
#include "monte_carlo.h"
#include "ray.h"
#include "transform.h"
#include "util.h"
#include "vector.h"

#include "json.hpp"
using json = nlohmann::json;

#include <fstream>
#include <iostream>

Color PathTracer::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    sample.eyePoints.push_back(intersection.point);

    Color result(0.f);
    if (m_bounceController.checkCounts(1)) {
        result = direct(intersection, scene, random, sample);
        sample.contributions.push_back({result, 1.f});
    }

    Color modulation = Color(1.f);
    Intersection lastIntersection = intersection;

    for (int bounce = 2; !m_bounceController.checkDone(bounce); bounce++) {
        BSDFSample bsdfSample = lastIntersection.material->sample(
            lastIntersection, random
        );
        Ray bounceRay(lastIntersection.point, bsdfSample.wi);

        Intersection bounceIntersection = scene.testIntersect(bounceRay);
        if (!bounceIntersection.hit) { break; }

        sample.eyePoints.push_back(bounceIntersection.point);

        const float invPDF = 1.f / bsdfSample.pdf;
        modulation *= bsdfSample.throughput
            * fmaxf(0.f, bsdfSample.wi.dot(lastIntersection.shadingNormal))
            * invPDF;
        lastIntersection = bounceIntersection;

        if (m_bounceController.checkCounts(bounce)) {
            const Color previous = result;

            Color Ld = direct(bounceIntersection, scene, random, sample);
            result += Ld * modulation;

            sample.contributions.push_back({result - previous, invPDF});
        }
    }

    return result;
}

Color PathTracer::direct(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
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
        sample.shadowTests.push_back({
            intersection.point,
            lightSample.point,
            true
        });

        return Color(0.f);
    }

    Ray shadowRay = Ray(intersection.point, wo);
    float lightDistance = lightDirection.length();
    bool occluded = scene.testOcclusion(shadowRay, lightDistance);

    sample.shadowTests.push_back({
        intersection.point,
        lightSample.point,
        occluded
    });

    if (occluded) {
        return Color(0.f);
    }

    float invPDF = lightSample.invPDF * lightCount;
    return light->biradiance(lightSample, intersection.point)
        * intersection.material->f(intersection, wo)
        * fmaxf(0.f, wo.dot(intersection.shadingNormal))
        * invPDF;
}

void PathTracer::debug(const Intersection &intersection, const Scene &scene) const
{
    const int phiSteps = 300;
    const int thetaSteps = 300;

    RandomGenerator random;
    Sample sample;
    int bounceCount = 5;

    Transform hemisphereToWorld = normalToWorldSpace(intersection.normal);

    json j;
    j["QueryPoint"] = {
        intersection.point.x(),
        intersection.point.y(),
        intersection.point.z()
    };
    j["Steps"] = { { "phi", phiSteps }, { "theta", thetaSteps } };
    j["Gt"] = json::array();

    for (int phiStep = 0; phiStep < phiSteps; phiStep++) {
        for (int thetaStep = 0; thetaStep < thetaSteps; thetaStep++) {
            float phi = M_TWO_PI * phiStep / phiSteps;
            float theta = (M_PI / 2.f) * thetaStep / thetaSteps;

            float y = cosf(theta);
            float x = sinf(theta) * cosf(phi);
            float z = sinf(theta) * sinf(phi);

            Vector3 wiHemisphere(x, y, z);
            Vector3 wiWorld = hemisphereToWorld.apply(wiHemisphere);

            Ray ray = Ray(intersection.point, wiWorld);
            const Intersection fisheyeIntersection = scene.testIntersect(ray);
            Color sampleL(0.f);
            if (fisheyeIntersection.hit) {
                sampleL = L(
                    fisheyeIntersection,
                    scene,
                    random,
                    sample
                );

                Color emit = fisheyeIntersection.material->emit();
                sampleL += emit;
            }

            // std::cout << "phi: " << phi << " theta: " << theta << std::endl;
            // std::cout << "x: " << x << " y: " << y << " z: " << z << std::endl;
            // std::cout << sampleL << std::endl;

            j["Gt"].push_back({
                { "wi", { x, y, z } },
                { "phiStep", phiStep },
                { "thetaStep", thetaStep },
                { "phi", phi },
                { "theta", theta },
                { "radiance", { sampleL.r(), sampleL.g(), sampleL.b() } },
                { "luminance", { sampleL.luminance() } }
            });
        }
    }

    std::ofstream jsonFile("live.json");
    jsonFile << j.dump(4) << std::endl;
    std::cout << "Wrote to live.json" << std::endl;
}
