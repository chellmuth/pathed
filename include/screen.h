#pragma once

#include "app_controller.h"
#include "canvas.h"
#include "image.h"
#include "render_status.h"

#include <nanogui/label.h>
#include <nanogui/screen.h>
#include <nanogui/widget.h>

#include <memory>

class GLWidget;

class RenderWidget : public nanogui::Widget {
public:
    RenderWidget(
        Widget *parent,
        Image &image,
        std::shared_ptr<AppController> controller,
        int width,
        int height
    );

    bool keyboardEvent(int key, int scancode, int action, int modifiers) override;

    void draw(NVGcontext *ctx) override;

private:
    nanogui::ref<Canvas> m_canvas;
    std::shared_ptr<AppController> m_controller;
};

class PathedScreen : public nanogui::Screen {
public:
    PathedScreen(
        Image &image,
        Scene &scene,
        std::shared_ptr<RenderStatus> renderStatus,
        std::shared_ptr<AppController> controller,
        int width,
        int height
    );

    bool keyboardEvent(int key, int scancode, int action, int modifiers) override;
    void draw(NVGcontext *ctx) override;

private:
    void reloadRadioButtons();

    nanogui::ref<GLWidget> m_glWidget;
    nanogui::ref<RenderWidget> m_renderWidget;
    nanogui::ref<nanogui::Widget> m_buttonsGroup;
    nanogui::ref<nanogui::Label> m_sampleLabel;

    std::shared_ptr<AppController> m_controller;
    std::shared_ptr<RenderStatus> m_renderStatus;
};
