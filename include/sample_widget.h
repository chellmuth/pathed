#pragma once

#include "color.h"
#include "gl_types.h"

#include <nanogui/button.h>
#include <nanogui/label.h>
#include <nanogui/textbox.h>
#include <nanogui/widget.h>

#include <functional>
#include <vector>

struct SampleWidgetProps {
    int sampleCount;
    int currentSample;
    int renderX;
    int renderY;

    DebugMode debugMode;

    std::vector<Color> contributions;
};

class PagerWidget;

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
    void updateContributions();
    void updateCoordinates();
    void updateButtonStates();
    void updateCaption();

    SampleWidgetProps m_props;
    std::function<void(int, int)> m_coordinateCallback;
    std::function<void(DebugMode)> m_debugModeCallback;

    nanogui::ref<PagerWidget> m_samplePager;
    nanogui::ref<nanogui::IntBox<unsigned int> > m_xCoordinate;
    nanogui::ref<nanogui::IntBox<unsigned int> > m_yCoordinate;

    nanogui::ref<nanogui::Button> m_localButton;
    nanogui::ref<nanogui::Button> m_sourceButton;
    nanogui::ref<nanogui::Button> m_hemisphereButton;

    nanogui::ref<nanogui::Widget> m_contributionsTable;
};

