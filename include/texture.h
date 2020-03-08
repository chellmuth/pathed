#pragma once

#include "albedo.h"
#include "color.h"
#include "intersection.h"

#include <string>

class Texture : public Albedo {
public:
    Texture(const std::string &texturePath);

    void load();
    Color lookup(const Intersection &intersection) const override;

protected:
    std::string m_texturePath;

private:
    unsigned char *m_data;
    int m_width;
    int m_height;
    int m_channels;
};
