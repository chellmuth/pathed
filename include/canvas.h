#pragma once

#include <nanogui/opengl.h>
#include <nanogui/glcanvas.h>

#include "image.h"
#include "shader.h"

class Canvas : public nanogui::GLCanvas {
public:
    Canvas(Widget *parent, Image &image, int width, int height);
    void init();
    virtual void drawGL() override;

    void syncTextureBuffer();
private:
    Shader mShader;
    GLuint mVertexArrayID, mVertexBufferID, mElementBufferID, mTextureID;

    Image &mImage;
    int mWidth, mHeight;
    int mTextureWidth, mTextureHeight;
    std::vector<unsigned char> mTextureBuffer;
};
