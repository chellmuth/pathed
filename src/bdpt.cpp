#include "bdpt.h"

#include "color.h"
#include "light.h"
#include "monte_carlo.h"
#include "ray.h"
#include "transform.h"
#include "vector.h"

#include <assert.h>
#include <vector>

struct PathPoint {
    Point3 point;
    Vector3 normal;
    Material *material;
};

static bool visibilityTerm(const Scene &scene, const Point3 &p0, const Point3 &p1)
{
    Vector3 direction = (p1 - p0).toVector();
    Ray ray(p0, direction.normalized());
    Intersection intersection = scene.testIntersect(ray);
    assert(intersection.hit);

    return (direction.length() - intersection.t) < 1e-4;
}

static float geometryTerm(
    const Scene &scene,
    const Point3 &p0, const Vector3 &normal0,
    const Point3 &p1, const Vector3 &normal1
) {
    if (!visibilityTerm(scene, p0, p1)) { return 0.f; }

    Vector3 direction = (p1 - p0).toVector();
    Vector3 p0Out = direction.normalized();
    Vector3 p1Out = direction * -1.f;

    float numerator = fmaxf(0.f, normal0.dot(p0Out)) * fmaxf(0.f, normal1.dot(p1Out));
    float denominator = powf(direction.length(), 2);

    return numerator / denominator;
}

static Color pathThroughput(const Scene &scene, const std::vector<PathPoint> &path)
{
    for (int i = 1; i < path.size() - 1; i++) {
        const auto &previous = path[i - 1];
        const auto &current = path[i];
        const auto &next = path[i + 1];

        const Vector3 wi = (previous.point - current.point).toVector();
        const Vector3 wo = (next.point - current.point).toVector();

        Color brdf = current.material->f(wo, wi, current.normal);
        float geometry = geometryTerm(
            scene,
            current.point, current.normal,
            next.point, next.normal
        );
    }

    return Color(0.f, 0.f, 0.f);
}

Color BDPT::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    int bounceCount,
    Sample &sample
) const {
    sample.bounceRays.push_back(intersection.point);

    LightSample lightSample = scene.sampleLights(random);
    Vector3 hemisphereSample = UniformSampleHemisphere(random);
    Transform hemisphereToWorld = normalToWorldSpace(lightSample.normal);
    Vector3 bounceDirection = hemisphereToWorld.apply(hemisphereSample);
    Ray lightRay(lightSample.point, bounceDirection);
    sample.lightRays.push_back(lightSample.point);
    sample.lightRays.push_back(lightRay.at(1.f));

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
