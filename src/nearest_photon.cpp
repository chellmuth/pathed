#include "nearest_photon.h"

#include "globals.h"
#include "job.h"
#include "light.h"
#include "monte_carlo.h"
#include "point.h"
#include "ray.h"
#include "transform.h"
#include "vector.h"

#include "json.hpp"
using json = nlohmann::json;

#include <fstream>
#include <iostream>

static const int photonMaxBounces = 6;

NearestPhoton::NearestPhoton()
    : m_dataSource(std::make_shared<DataSource>())
{}

void NearestPhoton::preprocess(const Scene &scene, RandomGenerator &random)
{
    const int photonSamples = g_job->photonSamples();
    for (int i = 0; i < photonSamples; i++) {
        LightSample lightSample = scene.sampleLights(random);

        Vector3 hemisphereSample = UniformSampleHemisphere(random);
        Transform hemisphereToWorld = normalToWorldSpace(lightSample.normal);
        Vector3 bounceDirection = hemisphereToWorld.apply(hemisphereSample);
        Ray lightRay(lightSample.point, bounceDirection);

        Color throughput = lightSample.light->getMaterial()->emit();
        for (int bounce = 0; bounce < photonMaxBounces; bounce++) {
            Intersection intersection = scene.testIntersect(lightRay);
            if (!intersection.hit) { break; }

            throughput *= fmaxf(0.f, intersection.wi.dot(intersection.normal * -1.f));
            if (throughput.isBlack()) { break; }

            if (bounce > 0) { // don't guide towards direct lights
                m_dataSource->points.push_back({
                    intersection.point.x(),
                    intersection.point.y(),
                    intersection.point.z(),
                    lightRay.origin(),
                    throughput
                });
            }

            hemisphereSample = UniformSampleHemisphere(random);
            hemisphereToWorld = normalToWorldSpace(intersection.normal);
            bounceDirection = hemisphereToWorld.apply(hemisphereSample);
            lightRay = Ray(intersection.point, bounceDirection);

            throughput *= intersection.material->f(intersection, bounceDirection);
        }
    }

    m_KDTree = new KDTree(3, *m_dataSource, nanoflann::KDTreeSingleIndexAdaptorParams(10));
}

Color NearestPhoton::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    Sample &sample
) const
{
    const int searchCount = 1;
    auto resultIndices = std::make_shared<std::vector<size_t>>(searchCount);
    std::vector<float> outDistanceSquared(searchCount);

    nanoflann::KNNResultSet<float> resultSet(searchCount);
    resultSet.init(resultIndices->data(), outDistanceSquared.data());

    Point3 intersectionPoint = intersection.point;
    float queryPoint[3] = {
        intersectionPoint.x(),
        intersectionPoint.y(),
        intersectionPoint.z()
    };

    m_KDTree->findNeighbors(resultSet, queryPoint, nanoflann::SearchParams());
    return Color(outDistanceSquared[0]);
}

void NearestPhoton::debug(const Intersection &intersection, const Scene &scene) const
{
    Point3 intersectionPoint = intersection.point;

    json j;
    j["QueryPoint"] = { intersectionPoint.x(), intersectionPoint.y(), intersectionPoint.z() };
    j["Results"] = json::array();
    for (auto &point : m_dataSource->points) {
        j["Results"].push_back({
            { "point", { point.x, point.y, point.z } },
            { "source", { point.source.x(), point.source.y(), point.source.z() } },
            { "throughput", { point.throughput.r(), point.throughput.g(), point.throughput.b() } },
        });
    }

    std::ofstream jsonFile("live-photons.json");
    jsonFile << j.dump(4) << std::endl;
    std::cout << "Wrote to live-photons.json" << std::endl;
}
