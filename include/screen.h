#pragma once

#include "app_controller.h"
#include "canvas.h"
#include "image.h"

#include <nanogui/screen.h>

#include <memory>

class GLApplication;

class RenderScreen : public nanogui::Screen {
public:
    RenderScreen(Image &image, std::shared_ptr<AppController> controller, int width, int height);

    bool keyboardEvent(int key, int scancode, int action, int modifiers) override;

    void draw(NVGcontext *ctx) override;

private:
    Canvas *m_canvas;
    std::shared_ptr<AppController> m_controller;
};

class DebugScreen : public nanogui::Screen {
public:
    DebugScreen(Scene &scene, std::shared_ptr<AppController> controller, int width, int height);

    bool keyboardEvent(int key, int scancode, int action, int modifiers) override;

private:
    GLApplication *m_glApplication;
    std::shared_ptr<AppController> m_controller;
};
