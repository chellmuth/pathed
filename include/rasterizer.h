#pragma once

#include "gl_lines.h"
#include "gl_points.h"
#include "gl_scene.h"
#include "integrator.h"
#include "point.h"
#include "scene.h"
#include "shader.h"
#include "vector.h"

#include <Eigen/Geometry>
#include <nanogui/opengl.h>
#include <nanogui/glcanvas.h>
#include <vector>

enum class Direction {
    Forward,
    Backward,
    Left,
    Right
};

class Rasterizer : public nanogui::GLCanvas {
public:
    Rasterizer(Widget *parent, Scene &scene, int width, int height);

    void init();
    virtual void drawGL() override;

    void reload();
    void setState(const Sample &sample);
    void move(Direction direction);

private:
    Scene &mScene;
    Point3 mOrigin;
    Point3 mInitialDirection;

    nanogui::Arcball mArcball;
    gl::Scene mGLScene;
    gl::Lines mGLLines;
    gl::Points mGLPoints;

    int mWidth, mHeight;
    Shader mShader;

    void calculateViewMatrix(GLfloat (&view)[4][4]);

    bool mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers) override;
    bool mouseMotionEvent(const Eigen::Vector2i &p, const Eigen::Vector2i &rel, int button, int modifiers) override;
};
