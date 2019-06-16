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
#include <iostream>
#include <vector>

// PointPaths start from the light source and go to the eye
// Forward PDF is light -> eye, Reverse PDF is eye -> light
// s = number of light vertices, t = number of eye vertices
// i = number of light vertices during MIS

enum PointType {
    Eye,
    Light,
    Vertex,
};

struct PathPoint {
    PointType pointType;

    Point3 point;
    Vector3 normal;
    float pdf;
    Material *material;

    PathPoint(
        PointType _pointType,
        Point3 _point,
        Vector3 _normal,
        float _pdf,
        Material *_material
    ) : pointType(_pointType),
        point(_point),
        normal(_normal),
        pdf(_pdf),
        material(_material)
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
    const PathPoint &p0, const PathPoint &p1
) {
    //old
    if (!visibilityTerm(scene, p0.point, p1.point)) { return 0.f; }

    Vector3 direction = (p1.point - p0.point).toVector();
    Vector3 p0Out = direction.normalized();
    Vector3 p1Out = direction * -1.f;

    float numerator = 1.f;
    if (!p0.normal.isZero()) {
        numerator *= fmaxf(0.f, p0.normal.dot(p0Out));
    }
    if (!p1.normal.isZero()) {
        numerator *= fmaxf(0.f, p1.normal.dot(p1Out));
    }

    float denominator = powf(direction.length(), 2);

    return numerator / denominator;
}

static Color pathThroughput(const Scene &scene, const std::vector<PathPoint> &path, bool debug = false)
{
    for (int i = 0; i < path.size() - 1; i++) {
        const auto &current = path[i];
        const auto &next = path[i + 1];

        if (!visibilityTerm(scene, current.point, next.point)) {
            return Color(0.f);
        }
    }

    Color throughput(1.f);

    if (debug) {
        std::cout << "Throughput terms: ";
    }

    for (int i = 1; i < path.size() - 1; i++) {
        const auto &previous = path[i - 1];
        const auto &current = path[i];
        const auto &next = path[i + 1];

        const Vector3 wi = (previous.point - current.point).toVector();
        const Vector3 wo = (next.point - current.point).toVector();

        Color brdf = current.material->f(wo, wi, current.normal);
        float geometry = geometryTerm(scene, previous, current);

        if (debug) {
            std::cout << "(" << brdf * geometry << ")" << std::endl;
        }

        throughput *= brdf * geometry;
    }

    if (debug) {
        std::cout << std::endl;
    }

    return throughput;
}

static Color pathPDF(const Scene &scene, const std::vector<PathPoint> &path, int i)
{
    Color throughput(1.f);
    int s = i;
    int t = path.size() - s;

    // for (int i = 1; i < path.size() - 1; i++) {
    //     const auto &previous = path[i - 1];
    //     const auto &current = path[i];
    //     const auto &next = path[i + 1];

    //     const Vector3 wi = (previous.point - current.point).toVector();
    //     const Vector3 wo = (next.point - current.point).toVector();

    //     Color brdf = current.material->f(wo, wi, current.normal);
    //     float geometry = geometryTerm(
    //         scene,
    //         current.point, current.normal,
    //         next.point, next.normal
    //     );

    //     throughput *= brdf * geometry;
    // }

    return throughput;
}

static float pathPDF(const std::vector<PathPoint> &path)
{
    float pdf = 1.f;

    for (int i = 0; i < path.size() - 1; i++) {
        const auto &current = path[i];

        pdf *= current.pdf;
    }

    return pdf;
}

static Color pathRadiance(const Scene &scene, const std::vector<PathPoint> &path, bool debug = false)
{
    const int minS = 1;
    const int minT = 2;

    int vertexCount = path.size();

    int maxS = vertexCount - minT;

    for (int s = minS; s < maxS; s++) {
        int t = vertexCount - s;
        assert(t >= minT);
    }

    Color emitted = path[0].material->emit();
    if (debug) {
        std::cout << "Emitted: " << emitted << std::endl;
        std::cout << "Throughput: " << pathThroughput(scene, path, true) << std::endl;
        std::cout << "PDF: " << pathPDF(path) << std::endl;
    }

    return emitted * pathThroughput(scene, path) / pathPDF(path);
}

static Color pathThroughputOld(const Scene &scene, const std::vector<PathPoint> &path, bool debug = false)
{
    Color throughput(1.f);

    if (debug) {
        std::cout << "Throughput terms: ";
    }

    for (int i = 1; i < path.size() - 1; i++) {
        const auto &previous = path[i - 1];
        const auto &current = path[i];
        const auto &next = path[i + 1];

        const Vector3 wi = (previous.point - current.point).toVector();
        const Vector3 wo = (next.point - current.point).toVector();

        Color brdf = current.material->f(wo, wi, current.normal);
        float geometry = geometryTerm(scene, current, next);

        if (debug) {
            std::cout << "(" << brdf * geometry << ")" << std::endl;
        }

        throughput *= brdf * geometry;
    }

    if (debug) {
        std::cout << std::endl;
    }

    return throughput;
}

static float pathPDFOld(const std::vector<PathPoint> &path)
{
    float pdf = 1.f;

    for (int i = 1; i < path.size(); i++) {
        const auto &current = path[i];

        pdf *= current.pdf;
    }

    return pdf;
}

static Color pathRadianceOld(const Scene &scene, const std::vector<PathPoint> &path, bool debug = false)
{
    Color emitted = path[path.size() - 1].material->emit();

    if (debug) {
        std::cout << "Emitted: " << emitted << std::endl;
        std::cout << "Throughput: " << pathThroughputOld(scene, path, true) << std::endl;
        std::cout << "PDF: " << pathPDFOld(path) << std::endl;
    }

    return emitted * pathThroughputOld(scene, path) / pathPDFOld(path);
}

Color BDPT::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    int bounceCount,
    Sample &sample,
    bool debug
) const {
    if (debug) {
        std::cout << "<Debug Mode>" << std::endl;
    }

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
        PointType::Eye,
        scene.getCamera()->getOrigin(),
        Vector3(0.f), // not needed!
        -1.f,         // not needed!
        nullptr       // not needed!
    );

    PathPoint eyeBouncePoint(
        PointType::Vertex,
        intersection.point,
        intersection.normal,
        1.f,
        intersection.material
    );

    PathPoint lightBouncePoint(
        PointType::Vertex,
        lightIntersection.point,
        lightIntersection.normal,
        UniformHemispherePdf(),
        lightIntersection.material
    );

    PathPoint lightPoint(
        PointType::Light,
        lightSample.point,
        lightSample.normal,
        1.f / (lightSample.invPDF * scene.lights().size()),
        lightSample.light->getMaterial().get()
    );

    std::vector<PathPoint> path3 = {
        lightPoint,
        lightBouncePoint,
        eyeBouncePoint,
        eyePoint,
    };

    std::vector<PathPoint> path2 = {
        lightPoint,
        eyeBouncePoint,
        eyePoint,
    };

    std::vector<PathPoint> path3Old = {
        eyePoint,
        eyeBouncePoint,
        lightBouncePoint,
        lightPoint,
    };

    std::vector<PathPoint> path2Old = {
        eyePoint,
        eyeBouncePoint,
        lightPoint,
    };

    if (debug) {
        std::cout << "Points:" << std::endl;
        for (auto &pp : path3) {
            std::cout << pp.point << std::endl;
        }
        std::cout << "Radiance New: " << pathRadiance(scene, path3) << std::endl;
        pathRadiance(scene, path3, true);

        std::cout << "Radiance Old: " << pathRadianceOld(scene, path3Old) << std::endl;
        pathRadianceOld(scene, path3Old, true);
    }

    return pathRadiance(scene, path3);
}
