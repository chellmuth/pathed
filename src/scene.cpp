#include "scene.h"

#include "camera.h"
#include "color.h"
#include "globals.h"
#include "intersection.h"
#include "ray.h"
#include "util.h"
#include "uv.h"
#include "world_frame.h"

#include <cmath>
#include <limits>

static int secretValue = 28;

void Scene::InitMyContext(MyContext *contextPtr) const
{
  rtcInitIntersectContext(&contextPtr->context);
  contextPtr->surfacesPtr = &m_surfaces;
}

Scene::Scene(
    NestedSurfaceVector surfaces,
    std::vector<std::shared_ptr<Light> > lights,
    std::shared_ptr<EnvironmentLight> environmentLight,
    std::shared_ptr<Camera> camera
)
    : m_surfaces(surfaces),
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

    const MyContext* context = (const MyContext*)args->context;

    RTCHit *hit = (RTCHit*)args->hit;
    if (hit == nullptr) { return; }

    const std::vector<std::vector<std::shared_ptr<Surface> > > *surfaces = context->surfacesPtr;
    const auto &surfacePtr = (*surfaces)[hit->geomID][hit->primID];
    if (surfacePtr->getMaterial()->isContainer()) {
        std::cout << "CONTAINER!" << std::endl;
    }
}

void Scene::registerOcclusionFilters() const
{
    for (int geomID = 0; geomID < m_surfaces.size(); geomID++) {
        RTCGeometry rtcGeometry = rtcGetGeometry(g_rtcScene, geomID);
        rtcSetGeometryIntersectFilterFunction(
            rtcGeometry,
            occlusionFilter
        );
    }
}

NestedSurfaceVector Scene::getSurfaces()
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

    MyContext context;
    InitMyContext(&context);
    // rtcInitIntersectContext(&context);

    rtcIntersect1(
        g_rtcScene,
        &context.context,
        &rayHit
    );

    const RTCHit &hit = rayHit.hit;

    // need parallel arrays of geometry and materials
    if (hit.geomID != RTC_INVALID_GEOMETRY_ID) {
        RTCGeometry geometry = rtcGetGeometry(g_rtcScene, hit.geomID);

        const auto &surfacePtr = m_surfaces[rayHit.hit.geomID][rayHit.hit.primID];
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
            ).normalized() * -1.f;
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
        return hit;
    } else {
        return IntersectionHelper::miss;
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
    const Intersection &intersection,
    RandomGenerator &random
) const
{
    const int lightCount = m_lights.size();
    const int lightIndex = (int)floorf(random.next() * lightCount);

    const std::shared_ptr<Light> light = m_lights[lightIndex];
    const SurfaceSample surfaceSample = light->sample(intersection, random);

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
    const float areaMeasurePDF = lightIntersection.surface->pdf(lightPoint) / lightCount;

    const Vector3 lightDirection = (referencePoint - lightPoint).toVector();
    const Vector3 lightWo = lightDirection.normalized();
    const float distance = lightDirection.length();

    const float distance2 = distance * distance;
    const float projectedArea = WorldFrame::cosTheta(lightIntersection.normal, lightWo);

    const float solidAngleMeasurePDF = areaMeasurePDF * distance2 / projectedArea;
    return solidAngleMeasurePDF;
}

Color Scene::environmentL(const Vector3 &direction) const
{
    if (m_environmentLight) {
        return m_environmentLight->emit(direction);
    }
    return Color(0.f);
}

float Scene::environmentPDF(const Vector3 &direction) const
{
    assert(m_environmentLight);
    return m_environmentLight->emitPDF(direction) / lights().size();
}
