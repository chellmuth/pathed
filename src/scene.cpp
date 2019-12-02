#include "scene.h"

#include "camera.h"
#include "color.h"
#include "globals.h"
#include "intersection.h"
#include "ray.h"
#include "util.h"
#include "uv.h"

#include <embree3/rtcore.h>

#include <cmath>
#include <limits>

Scene::Scene(
    std::vector<std::shared_ptr<Primitive> > primitives,
    std::vector<std::vector<std::shared_ptr<Surface> > > surfaces,
    std::vector<std::shared_ptr<Light> > lights,
    std::shared_ptr<EnvironmentLight> environmentLight,
    std::shared_ptr<Camera> camera
)
    : m_surfaces(surfaces),
      m_lights(lights),
      m_environmentLight(environmentLight),
      m_camera(camera),
      m_bvh(new BVH())
{
    printf("BAKING...\n");
    m_bvh->bake(primitives);
    printf("BAKED...\n");

    rtcCommitScene(g_rtcScene);
}

std::vector<std::vector<std::shared_ptr<Surface> > > Scene::getSurfaces()
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

    rayHit.ray.tnear = 1e-3f;
    rayHit.ray.tfar = 1e5f;

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

    const RTCHit &hit = rayHit.hit;

    // need parallel arrays of geometry and materials
    if (hit.geomID != RTC_INVALID_GEOMETRY_ID) {
        RTCGeometry geometry = rtcGetGeometry(g_rtcScene, hit.geomID);
        UV uv;
        rtcInterpolate0(
            geometry,
            hit.primID,
            hit.u,
            hit.v,
            RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,
            0,
            &uv.u,
            2
        );

        float normalRaw[3];
        rtcInterpolate0(
            geometry,
            hit.primID,
            hit.u,
            hit.v,
            RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,
            1,
            &normalRaw[0],
            3
        );

        Vector3 shadingNormal = Vector3(normalRaw[0], normalRaw[1], normalRaw[2]);
        Vector3 geometricNormal = Vector3(
            rayHit.hit.Ng_x,
            rayHit.hit.Ng_y,
            rayHit.hit.Ng_z
        ).normalized() * -1.f;

        if (shadingNormal.length() == 0.f) {
            shadingNormal = geometricNormal;
        }

        Intersection hit = {
            .hit = true,
            .t = rayHit.ray.tfar,
            .point = ray.at(rayHit.ray.tfar),
            .wo = -ray.direction(),
            .normal = geometricNormal,
            .shadingNormal = shadingNormal.normalized(),
            // .shadingNormal = geometricNormal,
            .uv = uv,
            .material = m_surfaces[rayHit.hit.geomID][rayHit.hit.primID]->getMaterial().get()
        };
        return hit;
    } else {
        return IntersectionHelper::miss;
    }

    // return m_bvh->testIntersect(ray);
}

bool Scene::testOcclusion(const Ray &ray, float maxT) const
{
    RTCRay rtcRay;
    rtcRay.org_x = ray.origin().x();
    rtcRay.org_y = ray.origin().y();
    rtcRay.org_z = ray.origin().z();

    rtcRay.dir_x = ray.direction().x();
    rtcRay.dir_y = ray.direction().y();
    rtcRay.dir_z = ray.direction().z();

    rtcRay.tnear = 1e-3f;
    rtcRay.tfar = maxT - 1e-3f;

    rtcRay.flags = 0;

    RTCIntersectContext context;
    rtcInitIntersectContext(&context);

    rtcOccluded1(
        g_rtcScene,
        &context,
        &rtcRay
    );

    return std::isinf(rtcRay.tfar);
}

LightSample Scene::sampleLights(RandomGenerator &random) const
{
    int lightCount = m_lights.size();
    int lightIndex = (int)floorf(random.next() * lightCount);

    std::shared_ptr<Light> light = m_lights[lightIndex];
    SurfaceSample surfaceSample = light->sampleEmit(random);
    LightSample lightSample(
        light,
        surfaceSample.point,
        surfaceSample.normal,
        surfaceSample.invPDF
    );
    return lightSample;
}

Color Scene::environmentL(const Vector3 &direction) const
{
    if (m_environmentLight) {
        return m_environmentLight->emit(direction);
    }
    return Color(0.f);
}
