#include "depositer.h"

#include "color.h"
#include "light.h"
#include "monte_carlo.h"
#include "ray.h"
#include "transform.h"
#include "vector.h"

#include <iostream>

using namespace std;
using namespace nanoflann;

void Depositer::preprocess(const Scene &scene, RandomGenerator &random)
{
    for (int i = 0; i < 1; i++) {
        LightSample lightSample = scene.sampleLights(random);

        Vector3 hemisphereSample = UniformSampleHemisphere(random);
        Transform hemisphereToWorld = normalToWorldSpace(lightSample.normal);
        Vector3 bounceDirection = hemisphereToWorld.apply(hemisphereSample);
        Ray lightRay(lightSample.point, bounceDirection);

        Intersection lightIntersection = scene.testIntersect(lightRay);
        if (!lightIntersection.hit) { continue; }

        mDataSource.pts.push_back({
            lightIntersection.point.x(),
            lightIntersection.point.y(),
            lightIntersection.point.z()
        });
    }

    mKDTree = new KDTree(3, mDataSource, KDTreeSingleIndexAdaptorParams(10));
}

Color Depositer::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    int bounceCount,
    Sample &sample
) const {
    Point3 intersectionPoint = intersection.point;
	float queryPoint[3] = { intersectionPoint.x(), intersectionPoint.y(), intersectionPoint.z() };

    const size_t numResults = 1;
    size_t returnIndex;
    float outDistanceSquared;
    nanoflann::KNNResultSet<float> resultSet(numResults);
    resultSet.init(&returnIndex, &outDistanceSquared);

    mKDTree->findNeighbors(resultSet, queryPoint, nanoflann::SearchParams());

    return Color(outDistanceSquared);

    // sample.eyePoints.push_back(intersection.point);

    // Color result = direct(intersection, scene, random, sample);

    // Color modulation = Color(1.f, 1.f, 1.f);
    // Intersection lastIntersection = intersection;

    // for (int bounce = 0; bounce < bounceCount; bounce++) {
    //     Transform hemisphereToWorld = normalToWorldSpace(
    //         lastIntersection.normal,
    //         lastIntersection.wi
    //     );

    //     Vector3 hemisphereSample = UniformSampleHemisphere(random);
    //     Vector3 bounceDirection = hemisphereToWorld.apply(hemisphereSample);
    //     Ray bounceRay(
    //         lastIntersection.point,
    //         bounceDirection
    //     );

    //     Intersection bounceIntersection = scene.testIntersect(bounceRay);
    //     if (!bounceIntersection.hit) { break; }

    //     sample.eyePoints.push_back(bounceIntersection.point);

    //     float pdf;
    //     Color f = lastIntersection.material->f(
    //         lastIntersection.wi,
    //         bounceDirection,
    //         lastIntersection.normal,
    //         &pdf
    //     );
    //     float invPDF = 1.f / pdf;

    //     modulation *= f
    //         * fmaxf(0.f, bounceDirection.dot(lastIntersection.normal))
    //         * invPDF;
    //     lastIntersection = bounceIntersection;

    //     result += direct(bounceIntersection, scene, random, sample) * modulation;
    // }

    // return result;
}

Color Depositer::direct(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const {
    Color emit = intersection.material->emit();
    if (!emit.isBlack()) {
        // part of my old logic - if you hit an emitter, don't do direct lighting?
        // sample.shadowRays.push_back(intersection.point);
        return Color(0.f, 0.f, 0.f);
    }

    int lightCount = scene.lights().size();
    int lightIndex = (int)floorf(random.next() * lightCount);

    std::shared_ptr<Light> light = scene.lights()[lightIndex];
    SurfaceSample lightSample = light->sample(random);

    Vector3 lightDirection = (lightSample.point - intersection.point).toVector();
    Vector3 wo = lightDirection.normalized();

    sample.shadowPoints.push_back(lightSample.point);

    if (lightSample.normal.dot(wo) >= 0.f) {
        return Color(0.f, 0.f, 0.f);
    }

    Ray shadowRay = Ray(intersection.point, wo);
    Intersection shadowIntersection = scene.testIntersect(shadowRay);
    float lightDistance = lightDirection.length();

    if (shadowIntersection.hit && shadowIntersection.t + 0.0001f < lightDistance) {
        return Color(0.f, 0.f, 0.f);
    }

    float invPDF = lightSample.invPDF * lightCount;

    return light->biradiance(lightSample, intersection.point)
        * intersection.material->f(intersection.wi, wo, intersection.normal)
        * fmaxf(0.f, wo.dot(intersection.normal))
        * invPDF;
}
