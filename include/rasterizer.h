#pragma once

#include "gl_lines.h"
#include "gl_scene.h"
#include "gl_visualization.h"
#include "integrator.h"
#include "point.h"
#include "photon_renderer.h"
#include "scene.h"
#include "shader.h"
#include "vector.h"

#include <Eigen/Geometry>
#include <nanogui/opengl.h>
#include <nanogui/glcanvas.h>

#include <memory>
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

    void setState(const Sample &sample);

    void move(Direction direction);
    void updateDebugMode();

    void setShowVisualization(bool showVisualization) { m_showVisualization = showVisualization; }
    void setVisualization(std::unique_ptr<gl::Visualization> visualization) { m_visualization = std::move(visualization); }

private:
    bool m_showVisualization;
    std::unique_ptr<gl::Visualization> m_visualization;

    Scene &m_scene;
    Point3 m_origin;
    Point3 m_initialDirection;

    nanogui::Arcball m_arcball;
    gl::Scene m_GLScene;
    gl::Lines m_GLLines;

    int m_width, m_height;
    Shader m_shader;

    void calculateViewMatrix(GLfloat (&view)[4][4]);

    bool mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers) override;
    bool mouseMotionEvent(const Eigen::Vector2i &p, const Eigen::Vector2i &rel, int button, int modifiers) override;
};
