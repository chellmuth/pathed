#include "sample_widget.h"

#include <nanogui/entypo.h>
#include <nanogui/layout.h>

#include <iostream>

using namespace nanogui;

class PagerWidget : public nanogui::Widget {
public:
    PagerWidget(
        Widget *parent,
        const PagerWidgetProps &props,
        std::function<void(int)> pagingCallback
    )
        : nanogui::Widget(parent),
          m_props(props),
          m_pagingCallback(pagingCallback)
    {
        auto layout = new BoxLayout(Orientation::Horizontal);
        layout->setSpacing(6);
        setLayout(layout);

        m_backButton = new Button(this, "", ENTYPO_ICON_ARROW_LEFT);
        m_backButton->setFixedWidth(40);
        m_backButton->setCallback([this] { m_pagingCallback(-1); });

        m_sampleLabel = new Label(this, "");

        m_forwardButton = new Button(this, "", ENTYPO_ICON_ARROW_RIGHT);
        m_forwardButton->setFixedWidth(40);
        m_forwardButton->setCallback([this] { m_pagingCallback(1); });

        update();
    }

    void update(const PagerWidgetProps &newProps) {
        m_props = newProps;
        update();
    }

private:
    void update() {
        m_backButton->setEnabled(m_props.currentCount > 0);
        m_forwardButton->setEnabled(m_props.currentCount + 1 < m_props.maxCount);

        std::string sampleCaption = m_props.caption + ": " + std::to_string(m_props.currentCount + 1);
        m_sampleLabel->setCaption(sampleCaption);
    }

    PagerWidgetProps m_props;

    std::function<void(int)> m_pagingCallback;

    nanogui::ref<nanogui::Label> m_sampleLabel;
    nanogui::ref<nanogui::Button> m_backButton;
    nanogui::ref<nanogui::Button> m_forwardButton;
};

SampleWidget::SampleWidget(
    Widget *parent,
    const SampleWidgetProps &props,
    std::function<void(int)> sampleCallback,
    std::function<void(int)> bounceCallback,
    std::function<void(int, int)> coordinateCallback,
    std::function<void(DebugMode)> debugModeCallback
) : nanogui::Widget(parent),
    m_props(props),
    m_coordinateCallback(coordinateCallback),
    m_debugModeCallback(debugModeCallback)
{
    auto layout = new BoxLayout(Orientation::Vertical);
    layout->setAlignment(Alignment::Fill);
    layout->setSpacing(10);
    setLayout(layout);

    m_samplePager = new PagerWidget(this, samplePagerProps(), sampleCallback);
    m_bouncePager = new PagerWidget(this, bouncePagerProps(), bounceCallback);

    {
        auto container = new Widget(this);
        auto containerLayout = new BoxLayout(Orientation::Horizontal);
        containerLayout->setSpacing(6);
        container->setLayout(containerLayout);

        m_xCoordinate = new IntBox<unsigned int>(container);
        m_xCoordinate->setEditable(true);
        m_xCoordinate->setFixedWidth(50);
        m_xCoordinate->setCallback([this](unsigned int x) {
            std::cout << "x: " << x << "!" << std::endl;
            m_coordinateCallback(x, m_props.renderY);
        });

        m_yCoordinate = new IntBox<unsigned int>(container);
        m_yCoordinate->setEditable(true);
        m_yCoordinate->setFixedWidth(50);
        m_yCoordinate->setCallback([this](unsigned int y) {
            std::cout << "y: " << y << "!" << std::endl;
            m_coordinateCallback(m_props.renderX, y);
        });
    }

    {
        auto container = new Widget(this);
        auto containerLayout = new BoxLayout(Orientation::Horizontal);
        containerLayout->setSpacing(6);
        container->setLayout(containerLayout);

        m_localButton = new Button(container, "Local");
        m_sourceButton = new Button(container, "Source");
        m_hemisphereButton = new Button(container, "Hemisphere");

        m_localButton->setFlags(Button::RadioButton);
        m_sourceButton->setFlags(Button::RadioButton);
        m_hemisphereButton->setFlags(Button::RadioButton);

        m_localButton->setCallback([this]() {
            m_debugModeCallback(DebugMode::Local);
        });
        m_sourceButton->setCallback([this]() {
            m_debugModeCallback(DebugMode::Source);
        });
        m_hemisphereButton->setCallback([this]() {
            m_debugModeCallback(DebugMode::Hemisphere);
        });
    }

    {
        auto container = new Widget(this);
        auto containerLayout = new BoxLayout(Orientation::Horizontal);
        containerLayout->setSpacing(6);
        container->setLayout(containerLayout);

        m_contributionsTable = new Widget(container);
        auto contributionsLayout = new BoxLayout(Orientation::Vertical);
        contributionsLayout->setSpacing(6);
        m_contributionsTable->setLayout(contributionsLayout);
    }

    updateContributions();
    updateCoordinates();
    updateButtonStates();
}

PagerWidgetProps SampleWidget::samplePagerProps()
{
    return {
        .currentCount = m_props.currentSample,
        .maxCount = m_props.sampleCount,
        .caption = "Sample"
    };
}

PagerWidgetProps SampleWidget::bouncePagerProps()
{
    return {
        .currentCount = m_props.currentBounce,
        .maxCount = m_props.bounceCount,
        .caption = "Bounce"
    };
}

void SampleWidget::update(const SampleWidgetProps &newProps)
{
    m_props = newProps;

    m_samplePager->update(samplePagerProps());
    m_bouncePager->update(bouncePagerProps());

    updateContributions();
    updateCoordinates();
    updateButtonStates();
}

void SampleWidget::updateCoordinates()
{
    m_xCoordinate->setValue(m_props.renderX);
    m_yCoordinate->setValue(m_props.renderY);
}

void SampleWidget::updateButtonStates()
{
    m_localButton->setPushed(m_props.debugMode == DebugMode::Local);
    m_sourceButton->setPushed(m_props.debugMode == DebugMode::Source);
    m_hemisphereButton->setPushed(m_props.debugMode == DebugMode::Hemisphere);
}

void SampleWidget::updateContributions()
{
    const int childCount = m_contributionsTable->childCount();
    for (int i = 0; i < childCount; i++) {
        m_contributionsTable->removeChild(0);
    }

    int i = 0;
    for (auto &contribution : m_props.contributions) {
        float luminance = contribution.radiance.luminance();
        float invPDF = contribution.invPDF;
        std::string text = "Bounce " + std::to_string(i) + ": " + std::to_string(luminance) + " invPDF: " + std::to_string(invPDF);
        auto contributionLabel = new Label(m_contributionsTable, text);
        i += 1;
    }
}
