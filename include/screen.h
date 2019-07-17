#pragma once

#include "canvas.h"
#include "image.h"
#include "kd_tree.h"
#include "render_status.h"
#include "sample_widget.h"
#include "scene.h"

#include <nanogui/label.h>
#include <nanogui/screen.h>
#include <nanogui/widget.h>

#include <functional>
#include <memory>
#include <vector>

class GLWidget;

class RenderWidget : public nanogui::Widget {
public:
    RenderWidget(
        Widget *parent,
        Image &image,
        std::function<void(int x, int y)> clickCallback,
        int width,
        int height
    );

    bool keyboardEvent(int key, int scancode, int action, int modifiers) override;
    bool mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers) override;

    void draw(NVGcontext *ctx) override;

private:
    nanogui::ref<Canvas> m_canvas;
    std::function<void(int x, int y)> m_clickCallback;
};

class PathedScreen : public nanogui::Screen {
public:
    PathedScreen(
        Image &image,
        Scene &scene,
        int width,
        int height
    );

    void updateRenderStatus(const RenderStatus &renderStatus);

    bool keyboardEvent(int key, int scancode, int action, int modifiers) override;

private:
    SampleWidgetProps m_sampleProps;

    void setCurrentSample(int currentSample, int renderX, int renderY, int bounce, DebugMode debugMode);
    const Sample& lookupSample(int currentSample, int renderX, int renderY);

    void reloadRadioButtons();
    void setPathVisualization();

    int m_width;
    int m_height;

    nanogui::ref<SampleWidget> m_sampleWidget;
    nanogui::ref<GLWidget> m_glWidget;
    nanogui::ref<RenderWidget> m_renderWidget;
    nanogui::ref<nanogui::Widget> m_buttonsGroup;
    nanogui::ref<nanogui::Label> m_sampleLabel;

    std::vector<std::shared_ptr<std::vector<Sample> > > m_sampleLookups;
    std::vector<std::shared_ptr<std::vector<DataSource::Point> > > m_photonLists;
    std::vector<std::shared_ptr<DataSource> > m_dataSources;
};
