#include "sample_widget.h"

#include <nanogui/entypo.h>
#include <nanogui/layout.h>

#include <iostream>

using namespace nanogui;

SampleWidget::SampleWidget(
    Widget *parent,
    const SampleWidgetProps &props,
    std::function<void(int)> sampleCallback,
    std::function<void(int, int)> coordinateCallback,
    std::function<void(DebugMode)> debugModeCallback
) : nanogui::Widget(parent),
    m_props(props),
    m_sampleCallback(sampleCallback),
    m_coordinateCallback(coordinateCallback),
    m_debugModeCallback(debugModeCallback)
{
    auto layout = new BoxLayout(Orientation::Vertical);
    layout->setAlignment(Alignment::Fill);
    layout->setSpacing(10);
    setLayout(layout);

    {
        auto container = new Widget(this);
        auto containerLayout = new BoxLayout(Orientation::Horizontal);
        containerLayout->setSpacing(6);
        container->setLayout(containerLayout);

        m_backButton = new Button(container, "", ENTYPO_ICON_ARROW_LEFT);
        m_backButton->setFixedWidth(40);
        m_backButton->setCallback([this] { m_sampleCallback(-1); });

        m_sampleLabel = new Label(container, "");

        m_forwardButton = new Button(container, "", ENTYPO_ICON_ARROW_RIGHT);
        m_forwardButton->setFixedWidth(40);
        m_forwardButton->setCallback([this] { m_sampleCallback(1); });
    }

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
    updateCaption();
}

void SampleWidget::update(const SampleWidgetProps &newProps)
{
    m_props = newProps;

    updateContributions();
    updateCoordinates();
    updateButtonStates();
    updateCaption();
}

void SampleWidget::updateCoordinates()
{
    m_xCoordinate->setValue(m_props.renderX);
    m_yCoordinate->setValue(m_props.renderY);
}

void SampleWidget::updateButtonStates()
{
    m_backButton->setEnabled(m_props.currentSample > 0);
    m_forwardButton->setEnabled(m_props.currentSample + 1 < m_props.sampleCount);

    m_localButton->setPushed(m_props.debugMode == DebugMode::Local);
    m_sourceButton->setPushed(m_props.debugMode == DebugMode::Source);
    m_hemisphereButton->setPushed(m_props.debugMode == DebugMode::Hemisphere);
}

void SampleWidget::updateCaption()
{
    std::string sampleCaption = "Sample: " + std::to_string(m_props.currentSample + 1);
    m_sampleLabel->setCaption(sampleCaption);
}

void SampleWidget::updateContributions()
{
    const int childCount = m_contributionsTable->childCount();
    for (int i = 0; i < childCount; i++) {
        m_contributionsTable->removeChild(0);
    }

    int i = 0;
    for (auto &contribution : m_props.contributions) {
        float luminance = contribution.luminance();
        std::string text = "Bounce " + std::to_string(i) + ": " + std::to_string(luminance);
        auto contributionLabel = new Label(m_contributionsTable, text);
        i += 1;
    }
}
