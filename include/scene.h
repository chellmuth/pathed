#pragma once

#include <list>

#include "json.hpp"
using json = nlohmann::json;

#include "intersection.h"
#include "sphere.h"

class Ray;

class Scene {
public:
    Scene(json sceneJson);

    Intersection testIntersect(const Ray &ray);

private:
    std::list<Sphere> m_objects;
};
