#pragma once

#include <memory>
#include <vector>

#include "bvh.h"
#include "environment_light.h"
#include "intersection.h"
#include "light.h"
#include "measure.h"
#include "point.h"
#include "primitive.h"
#include "random_generator.h"
#include "surface.h"
#include "world_frame.h"
#include "vector.h"

class Camera;
class Ray;

struct LightSample {
    std::shared_ptr<Light> light;
    Point3 point;
    Vector3 normal;
    float invPDF;
    Measure measure;

    LightSample(
        std::shared_ptr<Light> _light,
        Point3 _point,
        Vector3 _normal,
        float _invPDF
    ) : light(_light), point(_point), normal(_normal), invPDF(_invPDF),
        measure(Measure::Area)
    {}

    float solidAnglePDF(Point3 referencePoint)
    {
        if (measure == Measure::SolidAngle) {
            return 1.f / invPDF;
        }

        const Vector3 lightDirection = (point - referencePoint).toVector();
        const Vector3 lightWo = -lightDirection.normalized();
        const float projectedArea = WorldFrame::cosine(normal, lightWo);
        const float distance2 = lightDirection.length() * lightDirection.length();

        return (1.f / invPDF) * distance2 / projectedArea;
    }
};

class Scene {
public:
    Scene(
        // old
        std::vector<std::shared_ptr<Primitive> > primitives,
        std::vector<std::vector<std::shared_ptr<Surface> > > surfaces,
        std::vector<std::shared_ptr<Light> > lights,
        std::shared_ptr<EnvironmentLight> environmentLight,

        // new
        std::shared_ptr<Camera> camera
    );

    const std::vector<std::shared_ptr<Light>> &lights() const { return m_lights; }

    Intersection testIntersect(const Ray &ray) const;
    bool testOcclusion(const Ray &ray, float maxT) const;

    std::vector<std::vector<std::shared_ptr<Surface> > > getSurfaces();
    std::shared_ptr<Camera> getCamera() const { return m_camera; }

    LightSample sampleLights(RandomGenerator &random) const;
    Color environmentL(const Vector3 &direction) const;
    float environmentPDF(const Vector3 &direction) const;

private:
    std::unique_ptr<BVH> m_bvh;
    std::vector<std::vector<std::shared_ptr<Surface> > > m_surfaces;

    std::vector<std::shared_ptr<Light> > m_lights;
    std::shared_ptr<EnvironmentLight> m_environmentLight;

    std::shared_ptr<Camera> m_camera;
};
