#include "nearest_photon.h"

#include "light.h"
#include "monte_carlo.h"
#include "point.h"
#include "ray.h"
#include "transform.h"
#include "vector.h"

static const int photonSamples = 1e4;
static const int photonMaxBounces = 6;

NearestPhoton::NearestPhoton()
    : m_dataSource(std::make_shared<DataSource>())
{}

void NearestPhoton::preprocess(const Scene &scene, RandomGenerator &random)
{
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
