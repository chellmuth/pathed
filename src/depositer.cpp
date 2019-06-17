#include "depositer.h"

#include "color.h"
#include "light.h"
#include "monte_carlo.h"
#include "ray.h"
#include "transform.h"
#include "vector.h"

#include <iostream>
#include <vector>

using namespace std;
using namespace nanoflann;

template <typename T>
struct PointCloud
{
	struct Point
	{
		T  x,y,z;
	};

	std::vector<Point>  pts;

	// Must return the number of data points
	inline size_t kdtree_get_point_count() const { return pts.size(); }

	// Returns the dim'th component of the idx'th point in the class:
	// Since this is inlined and the "dim" argument is typically an immediate value, the
	//  "if/else's" are actually solved at compile time.
	inline T kdtree_get_pt(const size_t idx, const size_t dim) const
	{
		if (dim == 0) return pts[idx].x;
		else if (dim == 1) return pts[idx].y;
		else return pts[idx].z;
	}

	// Optional bounding-box computation: return false to default to a standard bbox computation loop.
	//   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
	//   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
	template <class BBOX>
	bool kdtree_get_bbox(BBOX& /* bb */) const { return false; }

};

void Depositer::preprocess(const Scene &scene, RandomGenerator &random)
{
    LightSample lightSample = scene.sampleLights(random);

    Vector3 hemisphereSample = UniformSampleHemisphere(random);
    Transform hemisphereToWorld = normalToWorldSpace(lightSample.normal);
    Vector3 bounceDirection = hemisphereToWorld.apply(hemisphereSample);
    Ray lightRay(lightSample.point, bounceDirection);

    Intersection lightIntersection = scene.testIntersect(lightRay);
    printf("%i\n", lightIntersection.hit);

	typedef KDTreeSingleIndexDynamicAdaptor<
		L2_Simple_Adaptor<float, PointCloud<float> > ,
		PointCloud<float>,
		3 /* dim */
		> my_kd_tree_t;

    PointCloud<float> cloud;
    cloud.pts.push_back({ lightSample.point.x(), lightSample.point.y(), lightSample.point.z() });

	float queryPoint[3] = { 0.f, 0.f, 0.f };

    my_kd_tree_t tree(3, cloud, KDTreeSingleIndexAdaptorParams(10));

    std::cout << "Searching..." << std::endl;
    const size_t numResults = 1;
    size_t returnIndex;
    float outDistanceSquared;
    nanoflann::KNNResultSet<float> resultSet(numResults);
    resultSet.init(&returnIndex, &outDistanceSquared);
    tree.findNeighbors(resultSet, queryPoint, nanoflann::SearchParams());
    std::cout << "return index=" << returnIndex << " outDistanceSquared=" << outDistanceSquared << std::endl;
    std::cout << "point: " << cloud.pts[returnIndex].x << ", " << cloud.pts[returnIndex].y << ", " << cloud.pts[returnIndex].z << std::endl;
}

Color Depositer::L(
    const Intersection &intersection,
    const Scene &scene,
    RandomGenerator &random,
    int bounceCount,
    Sample &sample
) const {
    sample.eyePoints.push_back(intersection.point);

    Color result = direct(intersection, scene, random, sample);

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

        sample.eyePoints.push_back(bounceIntersection.point);

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

        result += direct(bounceIntersection, scene, random, sample) * modulation;
    }

    return result;
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
