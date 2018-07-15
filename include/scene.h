#pragma once

#include "json.hpp"
using json = nlohmann::json;

#include "ray.h"
#include "sphere.h"

class Scene {
public:
    Scene(json sceneJson);

    bool testIntersect(const Ray &ray);

private:
    Sphere m_sphere;
};
