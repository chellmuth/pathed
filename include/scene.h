#pragma once

#include "json.hpp"
using json = nlohmann::json;

#include "intersection.h"
#include "ray.h"
#include "sphere.h"

class Scene {
public:
    Scene(json sceneJson);

    Intersection testIntersect(const Ray &ray);

private:
    Sphere m_sphere;
};
