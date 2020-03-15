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
    bool force() const {
        return m_json["force"].is_boolean()
            && m_json["force"].get<bool>();
    }

    int width() const { return m_json["width"].get<int>(); }
    int height() const { return m_json["height"].get<int>(); }

    int spp() const {
        int spp = m_json["spp"].get<int>();
        if (spp > 0) {
            return spp;
        }
        return 9999999;
    }

    int portOffset() const { return m_json["port_offset"].get<int>(); }
    int pdfSamples() const { return m_json["pdf_samples"].get<int>(); }

    std::string outputDirectory() const {
        return m_json["output_directory"].get<std::string>() + "/";
    }

    std::string outputName() const {
        return m_json["output_name"].get<std::string>();
    }

    std::string scene() const {
        return m_json["scene"].get<std::string>();
    }

    std::string visualizationDirectory() {
        return outputDirectory() + "visualization/";
    }

    std::string visualizationPath(const std::string &filename) {
        return visualizationDirectory() + filename;
    }

    int lightPhiSteps() const { return m_json["lightPhiSteps"].get<int>(); }
    int lightThetaSteps() const { return m_json["lightThetaSteps"].get<int>(); }
    int phiSteps() const { return m_json["phiSteps"].get<int>(); }
    int thetaSteps() const { return m_json["thetaSteps"].get<int>(); }
    int photonSamples() const { return m_json["photonSamples"].get<int>(); }
    int photonBounces() const { return m_json["photonBounces"].get<int>(); }
    int debugSearchCount() const { return m_json["debugSearchCount"].get<int>(); }

    int startBounce() const { return m_bounceController.startBounce(); }
    int lastBounce() const { return m_bounceController.lastBounce(); }
    BounceController bounceController() const { return m_bounceController; }

    std::shared_ptr<Integrator> integrator() const;

private:
    nlohmann::json m_json;
    BounceController m_bounceController;
};
