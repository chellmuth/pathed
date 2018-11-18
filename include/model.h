#pragma once

#include <memory>
#include <vector>

#include "color.h"
#include "surface.h"

class Model {
public:
    Model(std::vector<std::shared_ptr<Surface>> surfaces, Color diffuse, float specular);

private:
    std::vector<std::shared_ptr<Surface>> m_surfaces;

    Color m_diffuse;
    float m_specular;
};
