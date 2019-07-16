#pragma once

#include <nanogui/button.h>
#include <nanogui/label.h>
#include <nanogui/widget.h>

#include <functional>

struct SampleWidgetProps {
    int sampleCount;
    int currentSample;
    int renderX;
    int renderY;
};

class SampleWidget : public nanogui::Widget {
public:
    SampleWidget(
        Widget *parent,
        const SampleWidgetProps &props,
        std::function<void(int)> callback
    );
    void update(const SampleWidgetProps &props);

private:
    void updateButtonStates();
    void updateCaption();

    SampleWidgetProps m_props;
    std::function<void(int)> m_callback;

    nanogui::ref<nanogui::Label> m_sampleLabel;
    nanogui::ref<nanogui::Button> m_backButton;
    nanogui::ref<nanogui::Button> m_forwardButton;
};

