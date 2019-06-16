#pragma once

#include <memory>
#include <vector>

#include <Eigen/Geometry>
#include <nanogui/opengl.h>
#include <nanogui/glcanvas.h>

#include "app_controller.h"
#include "image.h"
#include "shader.h"

class Canvas : public nanogui::GLCanvas {
public:
    Canvas(Widget *parent, std::shared_ptr<AppController> controller, Image &image, int width, int height);

    void init();
    void save(char const *filestem, int spp);

    virtual void drawGL() override;

    void syncTextureBuffer();

private:
    virtual bool mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers) override;

    std::shared_ptr<AppController> mController;

    Shader mShader;
    GLuint mVertexArrayID, mVertexBufferID, mUVBufferID, mElementBufferID, mTextureID;

    Image &mImage;
    int mWidth, mHeight;
    int mTextureWidth, mTextureHeight;
    std::vector<unsigned char> mTextureBuffer;
};
