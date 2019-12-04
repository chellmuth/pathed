#include "pdf_widget.h"

#include "bounce_controller.h"
#include "camera.h"
#include "color.h"
#include "intersection.h"
#include "path_tracer.h"
#include "random_generator.h"
#include "ray.h"
#include "sample.h"
#include "transform.h"
#include "util.h"
#include "vector.h"

std::shared_ptr<Image> renderPDF(
    int phiSteps, int thetaSteps,
    int row, int col,
    const Scene &scene
) {
    std::shared_ptr<Image> image = std::make_shared<Image>(phiSteps, thetaSteps);

    const Ray ray = scene.getCamera()->generateRay((float)row, (float)col);
    const Intersection intersection = scene.testIntersect(ray);
    if (!intersection.hit) { return image; }

    BounceController bounceController(1, 1);
    PathTracer pathTracer(bounceController);

    RandomGenerator random;

    Transform hemisphereToWorld = normalToWorldSpace(
        intersection.normal,
        intersection.woWorld
    );

    const int spp = 16;

    #pragma omp parallel for
    for (int phiStep = 0; phiStep < phiSteps; phiStep++) {
        for (int thetaStep = 0; thetaStep < thetaSteps; thetaStep++) {
            Color sampleL(0.f);

            for (int i = 0; i < spp; i++) {
                float phi = M_TWO_PI * (phiStep + random.next()) / phiSteps;
                float theta = (M_PI / 2.f) * (thetaStep + random.next()) / thetaSteps;

                float y = cosf(theta);
                float x = sinf(theta) * cosf(phi);
                float z = sinf(theta) * sinf(phi);

                Vector3 wiHemisphere(x, y, z);
                Vector3 wiWorld = hemisphereToWorld.apply(wiHemisphere);

                Ray ray = Ray(intersection.point, wiWorld);
                const Intersection fisheyeIntersection = scene.testIntersect(ray);
                if (fisheyeIntersection.hit) {
                    Sample dummySample;

                    sampleL += pathTracer.L(
                        fisheyeIntersection,
                        scene,
                        random,
                        dummySample
                    ) / spp;
                }
            }

            const float average = sampleL.average();
            image->set(thetaStep, phiStep, average, average, average);
        }
    }

    return image;
}
