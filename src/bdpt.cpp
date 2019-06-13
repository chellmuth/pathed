#include "bdpt.h"

#include "camera.h"
#include "color.h"
#include "light.h"
#include "material.h"
#include "monte_carlo.h"
#include "ray.h"
#include "transform.h"
#include "vector.h"

#include <assert.h>
#include <vector>

struct PathPoint {
    Point3 point;
    Vector3 normal;
    float pdf;
    Material *material;

    PathPoint(Point3 _point, Vector3 _normal, float _pdf, Material *_material)
        : point(_point), normal(_normal), pdf(_pdf), material(_material)
    {}
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
    Color throughput(1.f);

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

        throughput *= brdf * geometry;
    }

    return throughput;
}

static float pathPDF(const std::vector<PathPoint> &path)
{
    float pdf = 1.f;

    for (int i = 1; i < path.size(); i++) {
        const auto &current = path[i];

        pdf *= current.pdf;
    }

    return pdf;
}

static Color pathRadiance(const Scene &scene, const std::vector<PathPoint> &path)
{
    Color emitted = path[path.size() - 1].material->emit();
    return emitted * pathThroughput(scene, path) / pathPDF(path);
}

Color BDPT::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    int bounceCount,
    Sample &sample
) const {
    sample.eyePoints.push_back(intersection.point);

    LightSample lightSample = scene.sampleLights(random);
    sample.lightPoints.push_back(lightSample.point);
    Vector3 hemisphereSample = UniformSampleHemisphere(random);
    Transform hemisphereToWorld = normalToWorldSpace(lightSample.normal);
    Vector3 bounceDirection = hemisphereToWorld.apply(hemisphereSample);
    Ray lightRay(lightSample.point, bounceDirection);

    Intersection lightIntersection = scene.testIntersect(lightRay);
    if (!lightIntersection.hit) {
        sample.lightPoints.push_back(lightRay.at(10.f));
        sample.connected = false;

        return Color(0.f);
    }

    sample.lightPoints.push_back(lightIntersection.point);
    sample.connected = true;

    PathPoint eyePoint(
        scene.getCamera()->getOrigin(),
        Vector3(0.f), // not needed!
        -1.f,         // not needed!
        nullptr       // not needed!
    );

    PathPoint eyeBouncePoint(
        intersection.point,
        intersection.normal,
        1.f,
        intersection.material
    );

    PathPoint lightBouncePoint(
        lightIntersection.point,
        lightIntersection.normal,
        UniformHemispherePdf(),
        lightIntersection.material
    );

    PathPoint lightPoint(
        lightSample.point,
        lightSample.normal,
        1.f / lightSample.invPDF,
        lightSample.light->getMaterial().get()
    );

    std::vector<PathPoint> path = {
        eyePoint,
        eyeBouncePoint,
        lightBouncePoint,
        lightPoint
    };

    return pathRadiance(scene, path);
}
