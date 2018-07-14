#include <stdlib.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "image.h"

Image::Image(int width, int height)
    : m_height(height), m_width(width)
{
    m_data = (unsigned char *)malloc(m_height * m_width * 3 * sizeof(unsigned char));
}

Image::~Image()
{
    free(m_data);
}

void Image::set(int row, int col, float r, float g, float b)
{
    unsigned char byteR = r * 255;
    unsigned char byteG = g * 255;
    unsigned char byteB = b * 255;

    m_data[3 * (row * m_width + col) + 0] = byteR;
    m_data[3 * (row * m_width + col) + 1] = byteG;
    m_data[3 * (row * m_width + col) + 2] = byteB;
}

void Image::debug()
{
    for (int row = 0; row < m_height; row++) {
        for (int col = 0; col < m_width; col++) {
            printf(
                "(%d,%d,%d) ",
                (unsigned char)m_data[3 * (row * m_width + col) + 0],
                (unsigned char)m_data[3 * (row * m_width + col) + 1],
                (unsigned char)m_data[3 * (row * m_width + col) + 2]
            );
        }
        printf("\n");
    }
}


void Image::write(char const *filename)
{
    stbi_write_bmp(filename, m_width, m_height, 3, m_data);
}
