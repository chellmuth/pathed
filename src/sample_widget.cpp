#include "sample_widget.h"

#include <nanogui/entypo.h>
#include <nanogui/layout.h>

#include <iostream>

SampleWidget::SampleWidget(
    Widget *parent,
    const SampleWidgetProps &props,
    std::function<void(int)> sampleCallback,
    std::function<void(int, int)> coordinateCallback
) : nanogui::Widget(parent),
    m_props(props),
    m_sampleCallback(sampleCallback),
    m_coordinateCallback(coordinateCallback)
{
    using namespace nanogui;

    setLayout(new GroupLayout());

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

    updateCoordinates();
    updateButtonStates();
    updateCaption();
}

void SampleWidget::update(const SampleWidgetProps &newProps)
{
    m_props = newProps;

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
}

void SampleWidget::updateCaption()
{
    std::string sampleCaption = "Sample: " + std::to_string(m_props.currentSample + 1);
    m_sampleLabel->setCaption(sampleCaption);
}
