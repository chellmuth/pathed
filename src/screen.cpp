#include "screen.h"

#include "globals.h"
#include "gl_types.h"
#include "job.h"
#include "path_visualization.h"
#include "pdf_widget.h"
#include "photon_renderer.h"
#include "rasterizer.h"
#include "visualization.h"

#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/glcanvas.h>
#include <nanogui/imageview.h>
#include <nanogui/layout.h>
#include <nanogui/opengl.h>
#include <nanogui/window.h>

#include <iostream>
#include <sstream>

using namespace nanogui;
using namespace std;

static const int MaxSavedSamples = 16;

class GLWidget : public nanogui::Widget {
public:
    GLWidget(Widget *parent, Scene &scene, int width, int height)
        : nanogui::Widget(parent)
    {
        using namespace nanogui;

        setSize({width, height});

        m_rasterizer = new Rasterizer(this, scene, width, height);
        m_rasterizer->setSize({width, height});
        m_rasterizer->init();
        m_rasterizer->setBackgroundColor({100, 100, 100, 255});
    }

    void setShowVisualization(bool showVisualization) {
        m_rasterizer->setShowVisualization(showVisualization);
    }

    void setVisualization(std::unique_ptr<gl::Visualization> renderer) {
        m_rasterizer->setVisualization(std::move(renderer));
    }

    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {
        if (key == GLFW_KEY_W) {
            m_rasterizer->move(Direction::Forward);
        } else if (key == GLFW_KEY_S) {
            m_rasterizer->move(Direction::Backward);
        } else if (key == GLFW_KEY_A) {
            m_rasterizer->move(Direction::Left);
        } else if (key == GLFW_KEY_D) {
            m_rasterizer->move(Direction::Right);
        } else if (key == GLFW_KEY_M && action == GLFW_PRESS) {
            m_rasterizer->updateDebugMode();
        }

        return false;
    }

private:
    Rasterizer *m_rasterizer;
};


RenderWidget::RenderWidget(
    Widget *parent,
    Image &image,
    std::function<void(int x, int y)> clickCallback,
    int width,
    int height
)
    : nanogui::Widget(parent),
      m_clickCallback(clickCallback)
{
    using namespace nanogui;

    setSize({width, height});

    m_canvas = new Canvas(this, image, width, height);
    m_canvas->setSize({width, height});
    m_canvas->init();
    m_canvas->setBackgroundColor({100, 100, 100, 255});
}

bool RenderWidget::keyboardEvent(int key, int scancode, int action, int modifiers)
{
    if (Widget::keyboardEvent(key, scancode, action, modifiers)) {
        return true;
    }

    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        m_canvas->save("output");
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        setVisible(false);
        return true;
    }

    return false;
}

void RenderWidget::draw(NVGcontext *ctx)
{
    m_canvas->syncTextureBuffer();

    Widget::draw(ctx);
}

bool RenderWidget::mouseButtonEvent(const Eigen::Vector2i &p, int button, bool down, int modifiers)
{
    if (button == GLFW_MOUSE_BUTTON_1 && down) {
        m_clickCallback(p.x(), p.y());
        return true;
    }

    return false;
}

PathedScreen::PathedScreen(
    std::shared_ptr<Integrator> integrator,
    Image &image,
    Scene &scene,
    int width,
    int height
)
    : nanogui::Screen(Eigen::Vector2i(width + 300, height), "Pathed: " + g_job->outputDirectory(), false),
      m_width(width),
      m_height(height),
      m_sampleProps({
          .sampleCount = 0,
          .currentSample = 0,

          .bounceCount = 0,
          .currentBounce = 0,

          .renderX = 0,
          .renderY = 0,
          .debugMode = DebugMode::Local
      })
{
    setLayout(new BoxLayout(Orientation::Horizontal));

    Widget *leftPanel = new Widget(this);
    leftPanel->setLayout(new GroupLayout());
    leftPanel->setFixedSize(Eigen::Vector2i(300, height));

    Widget *rightPanel = new Widget(this);
    rightPanel->setSize(Eigen::Vector2i(width, height));

    auto sampleCallback = [this](int sampleOffset) {
        setCurrentSample(
            m_sampleProps.currentSample + sampleOffset,
            m_sampleProps.renderX,
            m_sampleProps.renderY,
            0,
            m_sampleProps.debugMode
        );
    };
    auto bounceCallback = [this](int bounceOffset) {
        setCurrentSample(
            m_sampleProps.currentSample,
            m_sampleProps.renderX,
            m_sampleProps.renderY,
            m_sampleProps.currentBounce + bounceOffset,
            m_sampleProps.debugMode
        );
    };

    auto coordinateCallback = [this](int renderX, int renderY) {
        setCurrentSample(
            0,
            std::min(std::max(0, renderX), m_width - 1),
            std::min(std::max(0, renderY), m_height - 1),
            0,
            m_sampleProps.debugMode
        );
    };
    auto debugModeCallback = [this](DebugMode debugMode) {
        setCurrentSample(
            m_sampleProps.currentSample,
            m_sampleProps.renderX,
            m_sampleProps.renderY,
            m_sampleProps.currentBounce,
            debugMode
        );
    };
    m_sampleWidget = new SampleWidget(
        leftPanel,
        m_sampleProps,
        sampleCallback,
        bounceCallback,
        coordinateCallback,
        debugModeCallback
    );

    m_glWidget = new GLWidget(rightPanel, scene, width, height);
    auto clickCallback = [=, &scene](int x, int y) {
        std::cout << "clicked x: " << x << " y: " << y << std::endl;

        integrator->helloWorld();

        if (false) {
            const int rows = 400;
            const int cols = 400;

            auto popup = new nanogui::Screen(
                Eigen::Vector2i(cols * 2 + 20, rows + 100), "", false
                );
            popup->setVisible(true);

            std::shared_ptr<Image> image = renderPDF(cols, rows, (400 - y - 1), x, scene);
            const std::vector<unsigned char> imageData = image->data();

            unsigned char *data = (unsigned char *)malloc(rows * cols * 4 * sizeof(char));
            for (int row = 0; row < rows; row++) {
                for (int col = 0; col < cols; col++) {
                    int sourceOffset = 3 * (row * cols + col);

                    int targetOffset = 4 * (row * cols + col);
                    data[targetOffset + 0] = imageData[sourceOffset + 0];
                    data[targetOffset + 1] = imageData[sourceOffset + 1];
                    data[targetOffset + 2] = imageData[sourceOffset + 2];
                    data[targetOffset + 3] = 255;
                }
            }

            const int phiSteps = 400;
            const int thetaSteps = 400;

            std::vector<float> pdfs = integrator->visualizePDF(thetaSteps, phiSteps, (400 - y - 1), x, scene);

            unsigned char *data2 = (unsigned char *)malloc(phiSteps * thetaSteps * 4 * sizeof(char));
            for (int thetaStep = 0; thetaStep < thetaSteps; thetaStep++) {
                for (int phiStep = 0; phiStep < phiSteps; phiStep++) {
                    int sourceOffset = thetaStep * phiSteps + phiStep;

                    int targetOffset = 4 * (thetaStep * phiSteps + phiStep);
                    data2[targetOffset + 0] = 255 * fminf(1.f, pdfs[sourceOffset + 0]);
                    data2[targetOffset + 1] = 255 * fminf(1.f, pdfs[sourceOffset + 1]);
                    data2[targetOffset + 2] = 255 * fminf(1.f, pdfs[sourceOffset + 2]);
                    data2[targetOffset + 3] = 255;
                }
            }

            int imageID1 = nvgCreateImageRGBA(popup->nvgContext(), rows, cols, 0, data);
            int imageID2 = nvgCreateImageRGBA(popup->nvgContext(), phiSteps, thetaSteps, 0, data2);

            auto window = new nanogui::Window(popup, "PDF");
            window->setPosition({0, 0});
            window->setLayout(new GridLayout(Orientation::Horizontal, 2));

            auto imageView1 = new nanogui::ImageView(window, imageID1);
            auto imageView2 = new nanogui::ImageView(window, imageID2);

            popup->performLayout();
        }

        setCurrentSample(
            0,
            x,
            y,
            0,
            m_sampleProps.debugMode
        );
    };
    m_renderWidget = new RenderWidget(rightPanel, image, clickCallback, width, height);

    m_renderWidget->setVisible(true);
    m_glWidget->setVisible(false);

    auto visualizationToggle = new CheckBox(leftPanel, "Debug Mode");
    visualizationToggle->setCallback([this](bool isChecked) {
        if (isChecked) {
            m_renderWidget->setVisible(false);
            m_glWidget->setVisible(true);
        } else {
            m_renderWidget->setVisible(true);
            m_glWidget->setVisible(false);
        }
    });

    m_sampleLabel = new Label(leftPanel, "Sample: 0");

    auto reloadButton = new Button(leftPanel, "Reload json");
    reloadButton->setCallback([this]() {
        reloadRadioButtons();
    });

    m_buttonsGroup = new Widget(leftPanel);
    m_buttonsGroup->setLayout(new GroupLayout());

    reloadRadioButtons();

    performLayout();
}

const Sample& PathedScreen::lookupSample(int currentSample, int renderX, int renderY)
{
    const int lookupY = m_height - renderY - 1;
    const auto &sampleLookup = *m_sampleLookups[currentSample];
    const Sample &sample = sampleLookup[lookupY * m_width + renderX];
    return sample;
}

void PathedScreen::setPathVisualization()
{
    auto pathVisualization = std::make_unique<gl::PathVisualization>();

    const Sample &sample = lookupSample(
        m_sampleProps.currentSample,
        m_sampleProps.renderX,
        m_sampleProps.renderY
    );

    const auto &photons = *m_photonLists[m_sampleProps.currentSample];
    const auto &dataSource = *m_dataSources[m_sampleProps.currentSample];
    pathVisualization->init(
        sample,
        m_sampleProps.currentBounce,
        photons,
        dataSource,
        m_sampleProps.debugMode
    );

    m_glWidget->setVisualization(std::move(pathVisualization));
}

void PathedScreen::reloadRadioButtons()
{
    std::vector<std::string> files = visualization::files();

    int childCount = m_buttonsGroup->childCount();
    for (int i = childCount - 1; i >= 0; i--) {
        m_buttonsGroup->removeChild(i);
    }

    for (auto &file : files) {
        Button *fileButton = new Button(m_buttonsGroup, file);
        fileButton->setFlags(Button::RadioButton);
        fileButton->setCallback([this, file](void) {
            auto renderer = std::make_unique<gl::PhotonRenderer>();
            renderer->init(g_job->visualizationPath(file));
            m_glWidget->setVisualization(std::move(renderer));
            // m_glWidget->setShowVisualization(true);
        });
    }

    performLayout();
}

void PathedScreen::setCurrentSample(
    int currentSample,
    int renderX,
    int renderY,
    int bounce,
    DebugMode debugMode
) {
    m_sampleProps.currentSample = currentSample;
    m_sampleProps.renderX = renderX;
    m_sampleProps.renderY = renderY;
    m_sampleProps.currentBounce = bounce;
    m_sampleProps.debugMode = debugMode;

    const Sample &sample = lookupSample(
        m_sampleProps.currentSample,
        m_sampleProps.renderX,
        m_sampleProps.renderY
    );
    m_sampleProps.contributions = sample.contributions;
    m_sampleProps.bounceCount = sample.eyePoints.size() - 1;

    m_sampleWidget->update(m_sampleProps);
    setPathVisualization();

    performLayout();
}

void PathedScreen::updateRenderStatus(const RenderStatus &renderStatus)
{
    std::ostringstream sampleStream;
    sampleStream << "Sample: " << renderStatus.sample();

    m_sampleLabel->setCaption(sampleStream.str());
    if (renderStatus.sample() > MaxSavedSamples) { return; }


    // Update model
    m_sampleLookups.push_back(renderStatus.sampleLookup());
    m_photonLists.push_back(renderStatus.photons());
    m_dataSources.push_back(renderStatus.dataSource());


    // Update current props
    m_sampleProps.sampleCount = m_sampleLookups.size();


    // Reload UI
    m_sampleWidget->update(m_sampleProps);
    performLayout();
}

bool PathedScreen::keyboardEvent(int key, int scancode, int action, int modifiers)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        setVisible(false);
        return true;
    }

    return m_glWidget->keyboardEvent(key, scancode, action, modifiers);
}
