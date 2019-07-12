#include "screen.h"

#include "globals.h"
#include "job.h"
#include "photon_renderer.h"
#include "rasterizer.h"
#include "visualization.h"

#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/glcanvas.h>
#include <nanogui/layout.h>
#include <nanogui/opengl.h>

#include <iostream>

using namespace nanogui;
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

        m_controller->addSubscriber([this] {
            std::cout << "GL OBSERVED!" << std::endl;
        });
    }

    void setShowVisualization(bool showVisualization) {
        m_rasterizer->setShowVisualization(showVisualization);
    }

    void setVisualization(std::unique_ptr<gl::PhotonRenderer> renderer) {
        m_rasterizer->setVisualization(std::move(renderer));
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
            // m_rasterizer->reload();
        }

        m_rasterizer->setState(m_controller->getSample());
        Widget::draw(ctx);
    }

private:
    Rasterizer *m_rasterizer;
    std::shared_ptr<AppController> m_controller;
};


RenderScreen::RenderScreen(
    Widget *parent,
    Image &image,
    std::shared_ptr<AppController> controller,
    int width,
    int height
)
    : nanogui::Widget(parent),
      m_controller(controller)
{
    using namespace nanogui;

    m_canvas = new Canvas(this, controller, image, width, height);
    m_canvas->setSize({width, height});
    m_canvas->init();
    m_canvas->setBackgroundColor({100, 100, 100, 255});

    // performLayout();
}

bool RenderScreen::keyboardEvent(int key, int scancode, int action, int modifiers)
{
    if (Widget::keyboardEvent(key, scancode, action, modifiers)) {
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

    Widget::draw(ctx);
}

DebugScreen::DebugScreen(
    Image &image,
    Scene &scene,
    std::shared_ptr<AppController> controller,
    int width,
    int height
)
    : nanogui::Screen(Eigen::Vector2i(width + 300, height), "Debug", false),
      m_controller(controller)
{
    setLayout(new BoxLayout(Orientation::Horizontal));

    Widget *leftPanel = new Widget(this);
    leftPanel->setLayout(new GroupLayout());
    leftPanel->setFixedSize(Eigen::Vector2i(300, height));

    Widget *rightPanel = new Widget(this);
    rightPanel->setSize(Eigen::Vector2i(width, height));

    m_renderApplication = new RenderScreen(rightPanel, image, controller, width, height);
    m_glApplication = new GLApplication(rightPanel, scene, controller, width, height);

    m_renderApplication->setVisible(true);
    m_glApplication->setVisible(false);

    auto visualizationToggle = new CheckBox(leftPanel, "Debug Mode");
    visualizationToggle->setCallback([this](bool isChecked) {
        if (isChecked) {
            m_renderApplication->setVisible(false);
            m_glApplication->setVisible(true);
        } else {
            m_renderApplication->setVisible(true);
            m_glApplication->setVisible(false);
        }
    });

    m_buttonsGroup = new Widget(leftPanel);
    m_buttonsGroup->setLayout(new GroupLayout());

    reloadRadioButtons();

    performLayout();

    m_controller->addSubscriber([this] {
        reloadRadioButtons();
    });
}

void DebugScreen::reloadRadioButtons()
{
    std::vector<std::string> files = visualization::files();

    int childCount = m_buttonsGroup->childCount();
    for (int i = childCount - 1; i >= 0; i--) {
        m_buttonsGroup->removeChild(i);
    }

    for (auto &file : files) {
        Button *fileButton = new Button(m_buttonsGroup, file);
        fileButton->setFlags(Button::RadioButton);
        fileButton->setCallback([this, file](void) {
            auto renderer = std::make_unique<gl::PhotonRenderer>();
            renderer->init(g_job->visualizationPath(file));
            m_glApplication->setVisualization(std::move(renderer));
            // m_glApplication->setShowVisualization(true);
        });
    }

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
