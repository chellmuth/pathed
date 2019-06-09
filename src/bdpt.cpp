#include "bdpt.h"

#include "color.h"
#include "light.h"
#include "monte_carlo.h"
#include "ray.h"
#include "transform.h"
#include "vector.h"

Color BDPT::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    int bounceCount,
    Sample &sample
) const {
    sample.bounceRays.push_back(intersection.point);

    Color result(0.f, 0.f, 0.f); // = direct(intersection, scene, random, sample);

    Color modulation = Color(1.f, 1.f, 1.f);
    Intersection lastIntersection = intersection;

    for (int bounce = 0; bounce < bounceCount; bounce++) {
        Transform hemisphereToWorld = normalToWorldSpace(
            lastIntersection.normal,
            lastIntersection.wi
        );

        Vector3 hemisphereSample = UniformSampleHemisphere(random);
        Vector3 bounceDirection = hemisphereToWorld.apply(hemisphereSample);
        Ray bounceRay(
            lastIntersection.point,
            bounceDirection
        );

        Intersection bounceIntersection = scene.testIntersect(bounceRay);
        if (!bounceIntersection.hit) { break; }

        sample.bounceRays.push_back(bounceIntersection.point);

        float pdf;
        Color f = lastIntersection.material->f(
            lastIntersection.wi,
            bounceDirection,
            lastIntersection.normal,
            &pdf
        );
        float invPDF = 1.f / pdf;

        modulation *= f
            * fmaxf(0.f, bounceDirection.dot(lastIntersection.normal))
            * invPDF;
        lastIntersection = bounceIntersection;

        // result += direct(bounceIntersection, scene, random, sample) * modulation;
    }

    return result;
}
