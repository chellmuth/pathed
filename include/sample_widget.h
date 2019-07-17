#pragma once

#include "gl_types.h"

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

    DebugMode debugMode;
};

class SampleWidget : public nanogui::Widget {
public:
    SampleWidget(
        Widget *parent,
        const SampleWidgetProps &props,
        std::function<void(int)> sampleCallback,
        std::function<void(int, int)> coordinateCallback,
        std::function<void(DebugMode)> debugModeCallback
    );
    void update(const SampleWidgetProps &newProps);

private:
    void updateCoordinates();
    void updateButtonStates();
    void updateCaption();

    SampleWidgetProps m_props;
    std::function<void(int)> m_sampleCallback;
    std::function<void(int, int)> m_coordinateCallback;
    std::function<void(DebugMode)> m_debugModeCallback;

    nanogui::ref<nanogui::Label> m_sampleLabel;
    nanogui::ref<nanogui::Button> m_backButton;
    nanogui::ref<nanogui::Button> m_forwardButton;
    nanogui::ref<nanogui::IntBox<unsigned int> > m_xCoordinate;
    nanogui::ref<nanogui::IntBox<unsigned int> > m_yCoordinate;

    nanogui::ref<nanogui::Button> m_localButton;
    nanogui::ref<nanogui::Button> m_sourceButton;
    nanogui::ref<nanogui::Button> m_hemisphereButton;
};

