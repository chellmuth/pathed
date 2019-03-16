#pragma once

#include <nanogui/opengl.h>
#include <nanogui/glcanvas.h>
#include <vector>

#include "gl_lines.h"
#include "gl_scene.h"
#include "point.h"
#include "scene.h"
#include "shader.h"
#include "vector.h"

class Rasterizer : public nanogui::GLCanvas {
public:
    Rasterizer(Widget *parent, Scene &scene, int width, int height);

    void init();
    virtual void drawGL() override;

    void setState(Point3 point, std::vector<Vector3> intersections);

private:
    Scene &mScene;

    gl::Scene mGLScene;
    gl::Lines mGLLines;

    int mWidth, mHeight;
    Shader mShader;
};
