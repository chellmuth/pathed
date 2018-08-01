#pragma once

#include "color.h"
#include "intersection.h"

class Scene;

class Material {
public:
    Material(Color diffuse);

    Color shade(const Intersection &intersection, const Scene &scene);

private:
    Color m_diffuse;
};
