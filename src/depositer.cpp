#include "depositer.h"

#include "color.h"
#include "light.h"
#include "monte_carlo.h"
#include "ray.h"
#include "transform.h"
#include "vector.h"

#include <limits>
#include <math.h>

void Depositer::preprocess(const Scene &scene, RandomGenerator &random)
{
    const int samples = 10000;
    const int bounces = 10;
    for (int i = 0; i < samples; i++) {
        LightSample lightSample = scene.sampleLights(random);

        Vector3 hemisphereSample = UniformSampleHemisphere(random);
        Transform hemisphereToWorld = normalToWorldSpace(lightSample.normal);
        Vector3 bounceDirection = hemisphereToWorld.apply(hemisphereSample);
        Ray lightRay(lightSample.point, bounceDirection);

        Color throughput = lightSample.light->getMaterial()->emit();
        for (int bounce = 0; bounce < bounces; bounce++) {
            Intersection intersection = scene.testIntersect(lightRay);
            if (!intersection.hit) { break; }

            mDataSource.points.push_back({
                intersection.point.x(),
                intersection.point.y(),
                intersection.point.z(),
                lightRay.origin(),
                throughput
            });

            hemisphereSample = UniformSampleHemisphere(random);
            hemisphereToWorld = normalToWorldSpace(intersection.normal);
            bounceDirection = hemisphereToWorld.apply(hemisphereSample);
            lightRay = Ray(intersection.point, bounceDirection);

            throughput *= intersection.material->f(
                intersection.wi,
                bounceDirection,
                intersection.normal
            );
        }
    }

    mKDTree = new KDTree(3, mDataSource, nanoflann::KDTreeSingleIndexAdaptorParams(10));
}

static Color average(const DataSource &dataSource, const size_t indices[], size_t size)
{
    if (size == 0) { return Color(0.f); }

    Color sum(0.f);
    for (int i = 0; i < size; i++) {
        sum += dataSource.points[indices[i]].throughput / size;
    }

    return sum;
}

static float max(const float values[], size_t size)
{
    if (size == 0) { return 0.f; }

    float max = std::numeric_limits<float>::lowest();
    for (int i = 0; i < size; i++) {
        max = fmaxf(max, values[i]);
    }

    return max;
}

Color Depositer::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    int bounceCount,
    Sample &sample
) const {
    Point3 intersectionPoint = intersection.point;
	float queryPoint[3] = {
        intersectionPoint.x(),
        intersectionPoint.y(),
        intersectionPoint.z()
    };

    const size_t numResults = 10;
    size_t returnIndex[numResults];
    float outDistanceSquared[numResults];
    nanoflann::KNNResultSet<float> resultSet(numResults);
    resultSet.init(returnIndex, outDistanceSquared);

    mKDTree->findNeighbors(resultSet, queryPoint, nanoflann::SearchParams());

    Color irradiance = average(mDataSource, returnIndex, numResults);
    return irradiance * intersection.material->f(
        intersection.wi,
        Vector3(0.f), // hack to get albedo since I know it's lambertian
        intersection.normal
    );
}
