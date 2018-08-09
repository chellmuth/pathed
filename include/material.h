#pragma once

#include "color.h"
#include "intersection.h"

class Scene;

class Material {
public:
    Material(Color diffuse, Color emit);

    Color shade(const Intersection &intersection, const Scene &scene) const;

private:
    Color m_diffuse;
    Color m_emit;
};
