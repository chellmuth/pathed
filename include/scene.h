#pragma once

#include <memory>
#include <vector>

#include "bvh.h"
#include "intersection.h"
#include "light.h"
#include "point.h"
#include "primitive.h"
#include "random_generator.h"
#include "surface.h"
#include "vector.h"

class Camera;
class Ray;

struct LightSample {
    std::shared_ptr<Light> light;
    Point3 point;
    Vector3 normal;
    float invPDF;

    LightSample(
        std::shared_ptr<Light> _light,
        Point3 _point,
        Vector3 _normal,
        float _invPDF
    ) : light(_light), point(_point), normal(_normal), invPDF(_invPDF)
    {}
};

class Scene {
public:
    Scene(
        // old
        std::vector<std::shared_ptr<Primitive>> primitives,
        std::vector<std::shared_ptr<Surface>> surfaces,
        std::vector<std::shared_ptr<Light>> lights,

        // new
        std::shared_ptr<Camera> camera
    );

    std::vector<std::shared_ptr<Light>> lights() const { return m_lights; }
    Intersection testIntersect(const Ray &ray) const;

    std::vector<std::shared_ptr<Surface>> getSurfaces();
    std::shared_ptr<Camera> getCamera() const { return m_camera; }

    LightSample sampleLights(RandomGenerator &random) const;

private:
    std::unique_ptr<BVH> m_bvh;
    std::vector<std::shared_ptr<Surface>> m_surfaces;
    std::vector<std::shared_ptr<Light>> m_lights;
    std::shared_ptr<Camera> m_camera;
};
