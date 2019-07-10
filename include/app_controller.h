#pragma once

#include "integrator.h"
#include "point.h"
#include "random_generator.h"
#include "scene.h"
#include "vector.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

class AppController {
public:
    AppController(Scene &scene, int width, int height);
    bool testAndClearUpdate();
    void handlePathTraceClick(int x, int y);
    Sample getSample() { return m_sample; }

    std::vector<std::string> visualizationFiles() { return m_visualizationFiles; }

    void addSubscriber(std::function<void()> subscriber) { m_subscribers.push_back(subscriber); }

private:
    std::vector<std::function<void()> > m_subscribers;
    std::vector<std::string> m_visualizationFiles;

    std::unique_ptr<Integrator> m_integrator;
    RandomGenerator m_random;
    Scene &m_scene;
    int m_width, m_height;
    Sample m_sample;

    bool m_hasUpdate;
};
