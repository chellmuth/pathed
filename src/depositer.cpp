#include "depositer.h"

#include "color.h"
#include "light.h"
#include "monte_carlo.h"
#include "path_tracer.h"
#include "ray.h"
#include "transform.h"
#include "util.h"
#include "vector.h"

#include "json.hpp"
using json = nlohmann::json;

#include <fstream>
#include <limits>
#include <iostream>
#include <math.h>
#include <utility>

static int photonSamples = 1e5;
static int maxBounces = 10;

static const float searchRadius = 2e-3;
static const int debugSearchCount = 100;

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

void Depositer::debug(const Intersection &intersection, const Scene &scene) const
{
    Point3 intersectionPoint = intersection.point;
    float queryPoint[3] = {
        intersectionPoint.x(),
        intersectionPoint.y(),
        intersectionPoint.z()
    };

    size_t resultIndices[debugSearchCount];
    float outDistanceSquared[debugSearchCount];

    nanoflann::KNNResultSet<float> resultSet(debugSearchCount);
    resultSet.init(resultIndices, outDistanceSquared);

    mKDTree->findNeighbors(resultSet, queryPoint, nanoflann::SearchParams());

    json j;
    j["QueryPoint"] = { intersectionPoint.x(), intersectionPoint.y(), intersectionPoint.z() };
    j["Results"] = json::array();
    for (int i = 0; i < debugSearchCount; i++) {
        auto &point = mDataSource.points[resultIndices[i]];
        j["Results"].push_back({
            { "point", { point.x, point.y, point.z } },
            { "source", { point.source.x(), point.source.y(), point.source.z() } },
            { "throughput", { point.throughput.r(), point.throughput.g(), point.throughput.b() } },
        });
    }

    std::ofstream jsonFile("live-photons.json");
    jsonFile << j.dump(4) << std::endl;
    std::cout << "Wrote to live-photons.json" << std::endl;

    // std::cout << j.dump(4) << std::endl;

    debug2(intersection, scene);
}

void Depositer::debug2(const Intersection &intersection, const Scene &scene) const
{
    const int phiSteps = 300;
    const int thetaSteps = 300;

    PathTracer integrator;
    RandomGenerator random;
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
                const int spp = 2000;
                for (int i = 0; i < spp; i++) {
                    Sample sample;

                    sampleL += integrator.L(
                        fisheyeIntersection,
                        scene,
                        random,
                        bounceCount,
                        sample
                    ) / spp;

                    Color emit = fisheyeIntersection.material->emit();
                    sampleL += emit / spp;
                }
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

    std::ofstream jsonFile("live-gt.json");
    jsonFile << j.dump(4) << std::endl;
    std::cout << "Wrote to live-gt.json" << std::endl;
}
