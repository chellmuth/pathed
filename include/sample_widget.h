#pragma once

#include <nanogui/button.h>
#include <nanogui/label.h>
#include <nanogui/textbox.h>
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
        std::function<void(int)> sampleCallback,
        std::function<void(int, int)> coordinateCallback
    );
    void update(const SampleWidgetProps &newProps);

private:
    void updateCoordinates();
    void updateButtonStates();
    void updateCaption();

    SampleWidgetProps m_props;
    std::function<void(int)> m_sampleCallback;
    std::function<void(int, int)> m_coordinateCallback;

    nanogui::ref<nanogui::Label> m_sampleLabel;
    nanogui::ref<nanogui::Button> m_backButton;
    nanogui::ref<nanogui::Button> m_forwardButton;
    nanogui::ref<nanogui::IntBox<unsigned int> > m_xCoordinate;
    nanogui::ref<nanogui::IntBox<unsigned int> > m_yCoordinate;
};

