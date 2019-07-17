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

    int bounceCount;
    int currentBounce;

    int renderX;
    int renderY;

    DebugMode debugMode;

    std::vector<Color> contributions;
};

struct PagerWidgetProps {
    int currentCount;
    int maxCount;
    std::string caption;
};

class PagerWidget;

class SampleWidget : public nanogui::Widget {
public:
    SampleWidget(
        Widget *parent,
        const SampleWidgetProps &props,
        std::function<void(int)> sampleCallback,
        std::function<void(int)> bounceCallback,
        std::function<void(int, int)> coordinateCallback,
        std::function<void(DebugMode)> debugModeCallback
    );
    void update(const SampleWidgetProps &newProps);

private:
    void updateContributions();
    void updateCoordinates();
    void updateButtonStates();
    void updateCaption();

    PagerWidgetProps samplePagerProps();
    PagerWidgetProps bouncePagerProps();

    SampleWidgetProps m_props;
    std::function<void(int, int)> m_coordinateCallback;
    std::function<void(DebugMode)> m_debugModeCallback;

    nanogui::ref<PagerWidget> m_samplePager;
    nanogui::ref<PagerWidget> m_bouncePager;
    nanogui::ref<nanogui::IntBox<unsigned int> > m_xCoordinate;
    nanogui::ref<nanogui::IntBox<unsigned int> > m_yCoordinate;

    nanogui::ref<nanogui::Button> m_localButton;
    nanogui::ref<nanogui::Button> m_sourceButton;
    nanogui::ref<nanogui::Button> m_hemisphereButton;

    nanogui::ref<nanogui::Widget> m_contributionsTable;
};

