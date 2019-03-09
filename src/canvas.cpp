#define  GL_SILENCE_DEPRECATION 1

#include "canvas.h"

#include <iostream>
#include <mutex>
#include <vector>

Canvas::Canvas(Widget *parent, Image &image, int width, int height)
    : mImage(image), nanogui::GLCanvas(parent)
{
    mWidth = width;
    mHeight = height;

    mTextureWidth = mWidth;
    mTextureHeight = mHeight;
    mTextureBuffer.resize(mTextureWidth * mTextureHeight * 3);

    mShader = shader::createProgram(
        "shader/quad_texture.vs",
        "shader/quad_texture.fs"
    );
}

void Canvas::init()
{
    // TEXTURE
    for (int row = 0; row < mTextureHeight; row++) {
        for (int col = 0; col < mTextureWidth; col++) {
            mTextureBuffer[3 * (row * mTextureWidth + col) + 0] = 255;
            mTextureBuffer[3 * (row * mTextureWidth + col) + 1] = 255;
            mTextureBuffer[3 * (row * mTextureWidth + col) + 2] = 0;
        }
    }

    syncTextureBuffer();

    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mTextureWidth, mTextureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, mTextureBuffer.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // GEOMETRY
    float vertices[] = {
         1.0f,  1.0f, -0.5f,  // top right
         1.0f, -1.0f, -0.5f,  // bottom right
        -1.0f, -1.0f, -0.5f,  // bottom left
        -1.0f,  1.0f, -0.5f,  // top left
    };

    float uvs[] = {
        1.0f, 1.0f, // top right
        1.0f, 0.0f, // bottom right
        0.0f, 0.0f, // bottom left
        0.0f, 1.0f, // top left
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3,
    };

    glGenVertexArrays(1, &mVertexArrayID);

    glBindVertexArray(mVertexArrayID);

    glGenBuffers(1, &mVertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &mUVBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, mUVBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

    glGenBuffers(1, &mElementBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void Canvas::syncTextureBuffer()
{
    std::mutex &lock = mImage.getLock();
    lock.lock();

    const std::vector<unsigned char> &renderedBuffer = mImage.data();

    for (int row = 0; row < mHeight; row++) {
        for (int col = 0; col < mWidth; col++) {
            const int targetIndex = 3 * (row * mTextureWidth + col);
            const int sourceIndex = 3 * (row * mWidth + col);
            mTextureBuffer[targetIndex + 0] = renderedBuffer[sourceIndex + 0];
            mTextureBuffer[targetIndex + 1] = renderedBuffer[sourceIndex + 1];
            mTextureBuffer[targetIndex + 2] = renderedBuffer[sourceIndex + 2];
        }
    }

    glBindTexture(GL_TEXTURE_2D, mTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mTextureWidth, mTextureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, mTextureBuffer.data());

    lock.unlock();
}

void Canvas::drawGL()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(mShader.programID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTextureID);

    glBindVertexArray(mVertexArrayID);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, mUVBufferID);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBufferID);
    glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}
