#pragma once

#include <nanogui/opengl.h>
#include <nanogui/glcanvas.h>

#include "gl_lines.h"
#include "gl_scene.h"
#include "point.h"
#include "scene.h"
#include "shader.h"

class Rasterizer : public nanogui::GLCanvas {
public:
    Rasterizer(Widget *parent, Scene &scene, int width, int height);

    void init();
    virtual void drawGL() override;

    void setSelectedPoint(Point3 point);

private:
    Scene &mScene;

    gl::Scene mGLScene;
    gl::Lines mGLLines;

    int mWidth, mHeight;
    Shader mShader;
};
