#include "depositer.h"

#include "color.h"
#include "light.h"
#include "monte_carlo.h"
#include "ray.h"
#include "transform.h"
#include "vector.h"

#include <limits>
#include <math.h>
#include <utility>

static int photonSamples = 1e6;
static int maxBounces = 10;

static const float searchRadius = 2e-3;

void Depositer::preprocess(const Scene &scene, RandomGenerator &random)
{
    for (int i = 0; i < photonSamples; i++) {
        LightSample lightSample = scene.sampleLights(random);

        Vector3 hemisphereSample = UniformSampleHemisphere(random);
        Transform hemisphereToWorld = normalToWorldSpace(lightSample.normal);
        Vector3 bounceDirection = hemisphereToWorld.apply(hemisphereSample);
        Ray lightRay(lightSample.point, bounceDirection);

        Color throughput = lightSample.light->getMaterial()->emit();
        for (int bounce = 0; bounce < maxBounces; bounce++) {
            Intersection intersection = scene.testIntersect(lightRay);
            if (!intersection.hit) { break; }

            throughput *= fmaxf(0.f, intersection.wi.dot(intersection.normal * -1.f));

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

static Color average(const DataSource &dataSource, const std::vector<std::pair<size_t, float> > &indicesDistances)
{
    size_t size = indicesDistances.size();

    if (size == 0) { return Color(0.f); }

    Color sum(0.f);
    for (int i = 0; i < size; i++) {
        size_t index = indicesDistances[i].first;
        sum += dataSource.points[index].throughput / size;
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

    // {
    //     const size_t numResults = 10;
    //     size_t returnIndex[numResults];
    //     float outDistanceSquared[numResults];

    //     nanoflann::KNNResultSet<float> resultSet(numResults);
    //     resultSet.init(returnIndex, outDistanceSquared);

    //     mKDTree->findNeighbors(resultSet, queryPoint, nanoflann::SearchParams());
    // }

    {
		std::vector<std::pair<size_t, float> > indicesDistances;
        nanoflann::RadiusResultSet<float, size_t> resultSet(searchRadius, indicesDistances);

		mKDTree->findNeighbors(resultSet, queryPoint, nanoflann::SearchParams());
        Color irradiance = average(mDataSource, indicesDistances);

        return irradiance * intersection.material->f(
            intersection.wi,
            Vector3(0.f), // hack to get albedo since I know it's lambertian
            intersection.normal
        );
    }
    return Color(0.f);
}
