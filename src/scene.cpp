#include "scene.h"

#include "camera.h"
#include "color.h"
#include "globals.h"
#include "intersection.h"
#include "ray.h"
#include "util.h"
#include "uv.h"
#include "world_frame.h"

#include <algorithm>
#include <cmath>
#include <limits>

void Scene::InitCustomRTCIntersectContext(
    CustomRTCIntersectContext *contextPtr,
    bool shouldIntersectPassthroughs
) const
{
    rtcInitIntersectContext(&contextPtr->context);
    contextPtr->rtcManagerPtr = m_rtcManagerPtr.get();
    contextPtr->shouldIntersectPassthroughs = shouldIntersectPassthroughs;
}

Scene::Scene(
    std::unique_ptr<RTCManager> rtcManagerPtr,
    std::vector<std::shared_ptr<Light> > lights,
    std::shared_ptr<EnvironmentLight> environmentLight,
    std::shared_ptr<Camera> camera
)
    : m_rtcManagerPtr(std::move(rtcManagerPtr)),
      m_lights(lights),
      m_environmentLight(environmentLight),
      m_camera(camera)
{
    registerOcclusionFilters();

    rtcCommitScene(g_rtcScene);
}

static void occlusionFilter(const RTCFilterFunctionNArguments *args)
{
    if (args->context == nullptr) { return; }

    CustomRTCIntersectContext *context = (CustomRTCIntersectContext *)args->context;
    if (context->shouldIntersectPassthroughs) { return; }

    RTCHit *hit = (RTCHit *)args->hit;
    if (hit == nullptr) { return; }

    const auto &surfacePtr = (hit->instID[0] == RTC_INVALID_GEOMETRY_ID)
        ? context->rtcManagerPtr->lookupSurface(hit->geomID, hit->primID)
        : context->rtcManagerPtr->lookupInstancedSurface(
            hit->geomID,
            hit->primID,
            hit->instID
        )
    ;

    if (!surfacePtr->getMaterial()->isContainer()) { return; }

    std::shared_ptr<Medium> mediumPtr = surfacePtr->getInternalMedium();
    if (!mediumPtr) { return; }

    RTCRay *ray = (RTCRay *)args->ray;
    VolumeEvent event({
        ray->tfar,
        mediumPtr
    });

    bool validEvent = true;
    for (const VolumeEvent &existingEvent : context->volumeEvents) {
        if (event.t == existingEvent.t) {
            validEvent = false;
        }
    }

    if (validEvent) {
        context->volumeEvents.push_back(event);
    }

    args->valid[0] = 0;
}

void Scene::registerOcclusionFilters() const
{
    m_rtcManagerPtr->registerFilters(occlusionFilter);
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

    CustomRTCIntersectContext context;
    InitCustomRTCIntersectContext(&context, true);

    rtcIntersect1(
        g_rtcScene,
        &context.context,
        &rayHit
    );

    const RTCHit &hit = rayHit.hit;

    // need parallel arrays of geometry and materials
    if (hit.geomID != RTC_INVALID_GEOMETRY_ID) {
        RTCGeometry geometry;
        std::shared_ptr<Surface> surfacePtr;
        if (rayHit.hit.instID[0] == RTC_INVALID_GEOMETRY_ID) {
            geometry = rtcGetGeometry(g_rtcScene, hit.geomID);
            surfacePtr = m_rtcManagerPtr->lookupSurface(
                rayHit.hit.geomID,
                rayHit.hit.primID
            );
        } else {
            geometry = m_rtcManagerPtr->lookupGeometry(hit.geomID, rayHit.hit.instID);
            surfacePtr = m_rtcManagerPtr->lookupInstancedSurface(
                rayHit.hit.geomID,
                rayHit.hit.primID,
                rayHit.hit.instID
            );
        }
        const auto &shapePtr = surfacePtr->getShape();

        UV uv;
        Vector3 geometricNormal(0.f, 0.f, 0.f);
        Vector3 shadingNormal(0.f, 0.f, 0.f);
        if (shapePtr->useBackwardsNormals()) {
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

            // if (surfacePtr->getFaceIndex() % 2 == 0) {
            //     uv.u = hit.u * 0.f + hit.v * 0.f + (1.f - hit.u - hit.v) * 1.f;
            //     uv.v = hit.u * 0.f + hit.v * 1.f + (1.f - hit.u - hit.v) * 0.f;
            // } else {
            //     uv.u = hit.u * 0.f + hit.v * 1.f + (1.f - hit.u - hit.v) * 1.f;
            //     uv.v = hit.u * 1.f + hit.v * 1.f + (1.f - hit.u - hit.v) * 0.f;
            // }
            // uv.u = 1.f - uv.u;


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

            shadingNormal = Vector3(normalRaw[0], normalRaw[1], normalRaw[2]);

            geometricNormal = Vector3(
                rayHit.hit.Ng_x,
                rayHit.hit.Ng_y,
                rayHit.hit.Ng_z
            ).normalized();
        } else {
            // spheres
            geometricNormal = Vector3(
                rayHit.hit.Ng_x,
                rayHit.hit.Ng_y,
                rayHit.hit.Ng_z
            ).normalized();
        }

        if (shadingNormal.length() == 0.f) {
            shadingNormal = geometricNormal;
        }

        if (surfacePtr->getMaterial()->doubleSided()) {
            if (geometricNormal.dot(-ray.direction()) < 0.f) {
                geometricNormal = -geometricNormal;
            }
            if (shadingNormal.dot(-ray.direction()) < 0.f) {
                shadingNormal = -shadingNormal;
            }
        }

        Intersection hit = {
            .hit = true,
            .t = rayHit.ray.tfar,
            .point = ray.at(rayHit.ray.tfar),
            .woWorld = -ray.direction(),
            .normal = geometricNormal,
            .shadingNormal = shadingNormal.normalized(),
            // .shadingNormal = geometricNormal,
            .uv = uv,
            .material = surfacePtr->getMaterial().get(),
            .surface = surfacePtr.get()
        };
        return hit;
    } else {
        return IntersectionHelper::miss;
    }
}

IntersectionResult Scene::testVolumetricIntersect(const Ray &ray) const
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

    CustomRTCIntersectContext context;
    InitCustomRTCIntersectContext(&context, false);

    rtcIntersect1(
        g_rtcScene,
        &context.context,
        &rayHit
    );

    const RTCHit &hit = rayHit.hit;

    // need parallel arrays of geometry and materials
    if (hit.geomID != RTC_INVALID_GEOMETRY_ID) {
        RTCGeometry geometry;
        std::shared_ptr<Surface> surfacePtr;
        if (rayHit.hit.instID[0] == RTC_INVALID_GEOMETRY_ID) {
            geometry = rtcGetGeometry(g_rtcScene, hit.geomID);
            surfacePtr = m_rtcManagerPtr->lookupSurface(
                rayHit.hit.geomID,
                rayHit.hit.primID
            );
        } else {
            geometry = m_rtcManagerPtr->lookupGeometry(hit.geomID, rayHit.hit.instID);
            surfacePtr = m_rtcManagerPtr->lookupInstancedSurface(
                rayHit.hit.geomID,
                rayHit.hit.primID,
                rayHit.hit.instID
            );
        }
        const auto &shapePtr = surfacePtr->getShape();

        UV uv;
        Vector3 geometricNormal(0.f, 0.f, 0.f);
        Vector3 shadingNormal(0.f, 0.f, 0.f);
        if (shapePtr->useBackwardsNormals()) {
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

            shadingNormal = Vector3(normalRaw[0], normalRaw[1], normalRaw[2]);

            geometricNormal = Vector3(
                rayHit.hit.Ng_x,
                rayHit.hit.Ng_y,
                rayHit.hit.Ng_z
            ).normalized();
        } else {
            // spheres
            geometricNormal = Vector3(
                rayHit.hit.Ng_x,
                rayHit.hit.Ng_y,
                rayHit.hit.Ng_z
            ).normalized();
        }

        if (shadingNormal.length() == 0.f) {
            shadingNormal = geometricNormal;
        }

        Intersection hit = {
            .hit = true,
            .t = rayHit.ray.tfar,
            .point = ray.at(rayHit.ray.tfar),
            .woWorld = -ray.direction(),
            .normal = geometricNormal,
            .shadingNormal = shadingNormal.normalized(),
            // .shadingNormal = geometricNormal,
            .uv = uv,
            .material = surfacePtr->getMaterial().get(),
            .surface = surfacePtr.get()
        };

        std::sort(
            context.volumeEvents.begin(),
            context.volumeEvents.end(),
            [](VolumeEvent ve1, VolumeEvent ve2) {
                return ve1.t < ve2.t;
            }
        );

        return IntersectionResult({
            hit,
            context.volumeEvents
        });
    } else {
        return IntersectionResult({
            IntersectionHelper::miss,
            context.volumeEvents
        });
    }
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

    CustomRTCIntersectContext context;
    InitCustomRTCIntersectContext(&context, false);

    rtcOccluded1(
        g_rtcScene,
        &context.context,
        &rtcRay
    );

    return std::isinf(rtcRay.tfar);
}

OcclusionResult Scene::testVolumetricOcclusion(const Ray &ray, float maxT) const
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

    CustomRTCIntersectContext context;
    InitCustomRTCIntersectContext(&context, false);

    rtcOccluded1(
        g_rtcScene,
        &context.context,
        &rtcRay
    );

    if (std::isinf(rtcRay.tfar)) {
        return OcclusionResult({ true });
    }

    std::sort(
        context.volumeEvents.begin(),
        context.volumeEvents.end(),
        [](VolumeEvent ve1, VolumeEvent ve2) {
            return ve1.t < ve2.t;
        }
    );

    return OcclusionResult({
        false,
        context.volumeEvents
    });
}

LightSample Scene::sampleLights(RandomGenerator &random) const
{
    const int lightCount = m_lights.size();
    const int lightIndex = (int)floorf(random.next() * lightCount);

    const std::shared_ptr<Light> light = m_lights[lightIndex];
    const SurfaceSample surfaceSample = light->sampleEmit(random);

    const float lightChoicePDF = 1.f / lightCount;

    LightSample lightSample(
        light,
        surfaceSample.point,
        surfaceSample.normal,
        surfaceSample.invPDF * (1.f / lightChoicePDF),
        surfaceSample.measure
    );
    return lightSample;
}

LightSample Scene::sampleDirectLights(
    const Point3 &point,
    RandomGenerator &random
) const
{
    const int lightCount = m_lights.size();
    const int lightIndex = (int)floorf(random.next() * lightCount);

    const std::shared_ptr<Light> light = m_lights[lightIndex];
    const SurfaceSample surfaceSample = light->sample(point, random);

    const float lightChoicePDF = 1.f / lightCount;

    LightSample lightSample(
        light,
        surfaceSample.point,
        surfaceSample.normal,
        surfaceSample.invPDF * (1.f / lightChoicePDF),
        surfaceSample.measure
    );
    return lightSample;
}

float Scene::lightsPDF(
    const Point3 &referencePoint,
    const Intersection &lightIntersection,
    Measure measure
) const
{
    assert(measure == Measure::SolidAngle);
    if (measure == Measure::Area) { return 0.f; }

    const Point3 lightPoint = lightIntersection.point;

    const int lightCount = m_lights.size();
    float measurePDF = lightIntersection.surface->pdf(lightPoint, referencePoint, measure);

    return measurePDF / lightCount;
}

Color Scene::environmentL(const Vector3 &direction) const
{
    if (m_environmentLight) {
        return m_environmentLight->emit(-direction);
    }
    return Color(0.f);
}

float Scene::environmentPDF(const Vector3 &direction, Measure measure) const
{
    if (measure != Measure::SolidAngle) {
        throw std::runtime_error("Unsupported measure");
    }

    assert(m_environmentLight);
    return m_environmentLight->emitPDF(direction, measure) / lights().size();
}
