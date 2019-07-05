#pragma once

#include "bounce_controller.h"
#include "integrator.h"

#include "json.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

class Job {
public:
    Job(std::ifstream &jobFile);
    void init();

    bool showUI() const { return m_json["showUI"].get<bool>(); }

    int width() const { return m_json["width"].get<int>(); }
    int height() const { return m_json["height"].get<int>(); }

    std::string outputDirectory() const {
        return m_json["output_directory"].get<std::string>();
    }

    std::string outputName() const {
        return m_json["output_name"].get<std::string>();
    }

    std::string scene() const {
        return m_json["scene"].get<std::string>();
    }

    int phiSteps() const { return m_json["phiSteps"].get<int>(); }
    int thetaSteps() const { return m_json["thetaSteps"].get<int>(); }
    int photonSamples() const { return m_json["photonSamples"].get<int>(); }
    int debugSearchCount() const { return m_json["debugSearchCount"].get<int>(); }

    int startBounce() const { return m_bounceController.startBounce(); }
    int lastBounce() const { return m_bounceController.lastBounce(); }
    BounceController bounceController() const { return m_bounceController; }

    std::unique_ptr<Integrator> integrator() const;

private:
    nlohmann::json m_json;
    BounceController m_bounceController;
};
