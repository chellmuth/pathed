#include "texture.h"

#include "stb_image.h"

#include <iostream>
#include <math.h>

Texture::Texture(const std::string &texturePath)
    : m_texturePath(texturePath)
{}

void Texture::load()
{
    int width, height, channels;

    unsigned char *data = stbi_load(
        m_texturePath.c_str(),
        &width, &height,
        &channels,
        3
    );

    if (data) {
        m_data = data;
        m_width = width;
        m_height = height;
    } else {
        std::cout << m_texturePath << std::endl;
        throw "Error loading texture";
    }
}

Color Texture::lookup(UV uv) const
{
    // Handle wrapping
    float u = uv.u - (int)floorf(uv.u);
    float v = uv.v - (int)floorf(uv.v);

    int x = (int)roundf(u * (m_width - 1));
    int y = (int)roundf(v * (m_height - 1));

    return Color(
        m_data[3 * (y * m_width + x) + 0] / 255.f,
        m_data[3 * (y * m_width + x) + 1] / 255.f,
        m_data[3 * (y * m_width + x) + 2] / 255.f
    );
}
