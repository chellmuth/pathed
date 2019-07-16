#include "sample_widget.h"

#include <nanogui/entypo.h>
#include <nanogui/layout.h>

SampleWidget::SampleWidget(
    Widget *parent,
    const SampleWidgetProps &props,
    std::function<void(int)> callback
) : nanogui::Widget(parent),
    m_props(props),
    m_callback(callback)
{
    using namespace nanogui;

    setLayout(new GroupLayout());

    auto container = new Widget(this);
    auto containerLayout = new BoxLayout(Orientation::Horizontal);
    containerLayout->setSpacing(6);
    container->setLayout(containerLayout);

    m_backButton = new Button(container, "", ENTYPO_ICON_ARROW_LEFT);
    m_backButton->setFixedWidth(40);
    m_backButton->setCallback([this] { m_callback(-1); });

    m_sampleLabel = new Label(container, "");

    m_forwardButton = new Button(container, "", ENTYPO_ICON_ARROW_RIGHT);
    m_forwardButton->setFixedWidth(40);
    m_forwardButton->setCallback([this] { m_callback(1); });

    updateButtonStates();
    updateCaption();
}

void SampleWidget::update(const SampleWidgetProps &props)
{
    m_props = props;

    updateButtonStates();
    updateCaption();
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
