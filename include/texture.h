#pragma once

#include "color.h"
#include "uv.h"

#include <string>

class Texture {
public:
    Texture(const std::string &texturePath);

    void load();
    Color lookup(UV uv);

private:
    std::string m_texturePath;

    unsigned char *m_data;
    int m_width;
    int m_height;
    int m_channels;
};
