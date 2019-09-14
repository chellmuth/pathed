#pragma once

#include "image.h"
#include "shader.h"

#include <Eigen/Geometry>
#include <nanogui/opengl.h>
#include <nanogui/glcanvas.h>

#include <memory>
#include <string>
#include <vector>


class Canvas : public nanogui::GLCanvas {
public:
    Canvas(Widget *parent, Image &image, int width, int height);

    void init();
    void save(const std::string &filestem);

    virtual void drawGL() override;

    void syncTextureBuffer();

private:
    Shader mShader;
    GLuint mVertexArrayID, mVertexBufferID, mUVBufferID, mElementBufferID, mTextureID;

    Image &mImage;
    int mWidth, mHeight;
    int mTextureWidth, mTextureHeight;
    std::vector<unsigned char> mTextureBuffer;
};
