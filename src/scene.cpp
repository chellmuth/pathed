#include "scene.h"

#include "camera.h"
#include "color.h"
#include "globals.h"
#include "intersection.h"
#include "ray.h"
#include "util.h"

#include <embree3/rtcore.h>

#include <limits>

Scene::Scene(
    std::vector<std::shared_ptr<Primitive>> primitives,
    std::vector<std::shared_ptr<Surface>> surfaces,
    std::vector<std::shared_ptr<Light>> lights,
    std::shared_ptr<Camera> camera
)
    : m_surfaces(surfaces), m_lights(lights), m_camera(camera), m_bvh(new BVH())
{
    printf("BAKING...\n");
    m_bvh->bake(primitives);
    printf("BAKED...\n");
}

std::vector<std::shared_ptr<Surface>> Scene::getSurfaces()
{
    return m_surfaces;
}

Intersection Scene::testIntersect(const Ray &ray) const
{
    RTCRayHit rayHit;
    rayHit.ray.org_x = ray.origin().x();
    rayHit.ray.org_y = ray.origin().y();
    rayHit.ray.org_z = ray.origin().z();

    rayHit.ray.dir_x = ray.direction().x();
    rayHit.ray.dir_y = ray.direction().y();
    rayHit.ray.dir_z = ray.direction().z();

    rayHit.ray.tnear = 1e-5f;
    rayHit.ray.tfar = 1e5f;
    rayHit.ray.time = 0.f;

    rayHit.ray.flags = 0;

    rayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    rayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

    RTCIntersectContext context;
    rtcInitIntersectContext(&context);

    rtcIntersect1(
        g_rtcScene,
        &context,
        &rayHit
    );

    // need parallel arrays of geometry and materials
    if (rayHit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
        Intersection hit = {
            .hit = true,
            .t = rayHit.ray.tfar,
            .point = ray.at(rayHit.ray.tfar),
            .wi = ray.direction(),
            .normal = Vector3(
                rayHit.hit.Ng_x,
                rayHit.hit.Ng_y,
                rayHit.hit.Ng_z
            ).normalized() * -1.f,
            .uv = { 0.f, 0.f },
            .material = m_surfaces[0]->getMaterial().get()
        };
        return hit;
    } else {
        return IntersectionHelper::miss;
    }

    // return m_bvh->testIntersect(ray);
}

LightSample Scene::sampleLights(RandomGenerator &random) const
{
    int lightCount = m_lights.size();
    int lightIndex = (int)floorf(random.next() * lightCount);

    std::shared_ptr<Light> light = m_lights[lightIndex];
    SurfaceSample surfaceSample = light->sample(random);
    LightSample lightSample(
        light,
        surfaceSample.point,
        surfaceSample.normal,
        surfaceSample.invPDF
    );
    return lightSample;
}
