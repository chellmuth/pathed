#include "pdf_integrator.h"

#include "camera.h"
#include "color.h"
#include "globals.h"
#include "job.h"
#include "path_tracer.h"
#include "random_generator.h"
#include "ray.h"
#include "sample.h"
#include "transform.h"
#include "util.h"
#include "vector.h"

#include "json.hpp"
using json = nlohmann::json;

#include <fstream>
#include <iostream>

void PDFIntegrator::run(
    Image &image,
    Scene &scene,
    std::function<void(RenderStatus)> callback,
    bool *quit
) {
    Ray ray = scene.getCamera()->generateRay(400, 400);
    Intersection intersection = scene.testIntersect(ray);

    renderPDF(scene, intersection);

    *quit = true;
}

void PDFIntegrator::renderPDF(
    const Scene &scene,
    const Intersection &intersection
) {
    const int phiSteps = 300;
    const int thetaSteps = 300;

    PathTracer pathTracer(g_job->bounceController());

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
                sampleL = pathTracer.L(
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

