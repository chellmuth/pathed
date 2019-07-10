#include "screen.h"

#include "rasterizer.h"

#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/glcanvas.h>
#include <nanogui/layout.h>
#include <nanogui/opengl.h>

#include <iostream>

using namespace std;

class GLApplication : public nanogui::Widget {
public:
    GLApplication(Widget *parent, Scene &scene, std::shared_ptr<AppController> controller, int width, int height)
        : nanogui::Widget(parent),
          m_controller(controller)
    {
        using namespace nanogui;

        setSize({width, height});

        m_rasterizer = new Rasterizer(this, scene, width, height);
        m_rasterizer->setSize({width, height});
        m_rasterizer->init();
        m_rasterizer->setBackgroundColor({100, 100, 100, 255});
    }

    void setShowVisualization(bool showVisualization) {
        m_rasterizer->setShowVisualization(showVisualization);
    }

    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {
        if (key == GLFW_KEY_W) {
            m_rasterizer->move(Direction::Forward);
        } else if (key == GLFW_KEY_S) {
            m_rasterizer->move(Direction::Backward);
        } else if (key == GLFW_KEY_A) {
            m_rasterizer->move(Direction::Left);
        } else if (key == GLFW_KEY_D) {
            m_rasterizer->move(Direction::Right);
        } else if (key == GLFW_KEY_M && action == GLFW_PRESS) {
            m_rasterizer->updateDebugMode();
        }

        return false;
    }

    virtual void draw(NVGcontext *ctx) {
        if (m_controller->testAndClearUpdate()) {
            m_rasterizer->reload();
        }

        m_rasterizer->setState(m_controller->getSample());
        Widget::draw(ctx);
    }

private:
    Rasterizer *m_rasterizer;
    std::shared_ptr<AppController> m_controller;
};


RenderScreen::RenderScreen(
    Image &image, std::shared_ptr<AppController> controller, int width, int height
)
    : nanogui::Screen(Eigen::Vector2i(width, height), "Path Tracer", false),
    m_controller(controller)
{
    using namespace nanogui;

    m_canvas = new Canvas(this, controller, image, width, height);
    m_canvas->setSize({width, height});
    m_canvas->init();
    m_canvas->setBackgroundColor({100, 100, 100, 255});

    performLayout();
}

bool RenderScreen::keyboardEvent(int key, int scancode, int action, int modifiers)
{
    if (Screen::keyboardEvent(key, scancode, action, modifiers)) {
        return true;
    }

    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        m_canvas->save("output");
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        setVisible(false);
        return true;
    }

    return false;
}

void RenderScreen::draw(NVGcontext *ctx)
{
    m_canvas->syncTextureBuffer();

    Screen::draw(ctx);
}

DebugScreen::DebugScreen(
    Scene &scene,
    std::shared_ptr<AppController> controller,
    int width,
    int height
)
    : nanogui::Screen(Eigen::Vector2i(width + 300, height), "Debug", false)
{
    using namespace nanogui;

    setLayout(new BoxLayout(Orientation::Horizontal));

    Widget *leftPanel = new Widget(this);
    leftPanel->setLayout(new GroupLayout());
    leftPanel->setFixedSize(Eigen::Vector2i(300, height));

    Widget *rightPanel = new Widget(this);
    rightPanel->setSize(Eigen::Vector2i(width, height));

    m_glApplication = new GLApplication(rightPanel, scene, controller, width, height);

    auto visualizationToggle = new CheckBox(leftPanel, "Show Photons");
    visualizationToggle->setCallback([this](bool isChecked) {
        m_glApplication->setShowVisualization(isChecked);
    });

    Button *b = new Button(leftPanel, "Plain button");
    b->setCallback([] { cout << "pushed!" << endl; });
    b->setTooltip("short tooltip");

    performLayout();
}

bool DebugScreen::keyboardEvent(int key, int scancode, int action, int modifiers)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        setVisible(false);
        return true;
    }

    return m_glApplication->keyboardEvent(key, scancode, action, modifiers);
}
