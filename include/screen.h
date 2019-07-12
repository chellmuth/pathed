#pragma once

#include "app_controller.h"
#include "canvas.h"
#include "image.h"

#include <nanogui/screen.h>
#include <nanogui/widget.h>

#include <memory>

class GLApplication;

class RenderScreen : public nanogui::Widget {
public:
    RenderScreen(
        Widget *parent,
        Image &image,
        std::shared_ptr<AppController> controller,
        int width,
        int height
    );

    bool keyboardEvent(int key, int scancode, int action, int modifiers) override;

    void draw(NVGcontext *ctx) override;

private:
    Canvas *m_canvas;
    std::shared_ptr<AppController> m_controller;
};

class DebugScreen : public nanogui::Screen {
public:
    DebugScreen(
        Image &image,
        Scene &scene,
        std::shared_ptr<AppController> controller,
        int width,
        int height
    );

    bool keyboardEvent(int key, int scancode, int action, int modifiers) override;

private:
    void reloadRadioButtons();

    GLApplication *m_glApplication;
    std::shared_ptr<AppController> m_controller;
    nanogui::Widget *m_buttonsGroup;
};
