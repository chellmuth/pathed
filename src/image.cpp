#include "image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

#include <stdlib.h>

Image::Image(int width, int height)
    : m_height(height),
      m_width(width),
      m_data(3 * m_height * m_width),
      m_raw(3 * m_height * m_width)
{}

void Image::set(int row, int col, float r, float g, float b)
{
    m_raw[3 * (row * m_width + col) + 0] = r;
    m_raw[3 * (row * m_width + col) + 1] = g;
    m_raw[3 * (row * m_width + col) + 2] = b;

    unsigned char byteR = fminf(powf(r, 1/2.2), 1.f) * 255;
    unsigned char byteG = fminf(powf(g, 1/2.2), 1.f) * 255;
    unsigned char byteB = fminf(powf(b, 1/2.2), 1.f) * 255;

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

const std::vector<unsigned char> &Image::data()
{
    return m_data;
}

std::mutex &Image::getLock()
{
    return m_lock;
}

void Image::save()
{
    EXRHeader header;
    InitEXRHeader(&header);

    EXRImage image;
    InitEXRImage(&image);

    image.num_channels = 3;

    std::vector<float> images[3];
    images[0].resize(m_width * m_height);
    images[1].resize(m_width * m_height);
    images[2].resize(m_width * m_height);

    for (int i = 0; i < m_width * m_height; i++) {
        images[0][i] = m_raw[3 * i + 0];
        images[1][i] = m_raw[3 * i + 1];
        images[2][i] = m_raw[3 * i + 2];
    }

    float* image_ptr[3];
    image_ptr[0] = &(images[2].at(0)); // B
    image_ptr[1] = &(images[1].at(0)); // G
    image_ptr[2] = &(images[0].at(0)); // R

    image.images = (unsigned char**)image_ptr;
    image.width = m_width;
    image.height = m_height;

    header.num_channels = 3;
    header.channels = (EXRChannelInfo *)malloc(sizeof(EXRChannelInfo) * header.num_channels); 

    // Must be BGR(A) order, since most of EXR viewers expect this channel order.
    strncpy(header.channels[0].name, "B", 255); header.channels[0].name[strlen("B")] = '\0';
    strncpy(header.channels[1].name, "G", 255); header.channels[1].name[strlen("G")] = '\0';
    strncpy(header.channels[2].name, "R", 255); header.channels[2].name[strlen("R")] = '\0';

    header.pixel_types = (int *)malloc(sizeof(int) * header.num_channels); 
    header.requested_pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
    for (int i = 0; i < header.num_channels; i++) {
        header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
        header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in .EXR
    }

    const char* err;
    int ret = SaveEXRImageToFile(&image, &header, "test.exr", &err);
    if (ret != TINYEXR_SUCCESS) {
        fprintf(stderr, "Save EXR err: %s\n", err);
        return;
    }
    printf("Saved exr file. [ %s ] \n", "test.exr");

    free(header.channels);
    free(header.pixel_types);
    free(header.requested_pixel_types);
}

void Image::write(char const *filename)
{
    stbi_write_bmp(filename, m_width, m_height, 3, m_data.data());
}
