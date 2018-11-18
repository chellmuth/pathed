#include "model.h"

Model::Model(std::vector<std::shared_ptr<Surface>> surfaces, Color diffuse, float specular)
    : m_surfaces(surfaces), m_diffuse(diffuse), m_specular(specular)
{}
