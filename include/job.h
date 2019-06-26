#pragma once

#include "integrator.h"

#include "json.hpp"

#include <fstream>
#include <iostream>
#include <memory>

class Job {
public:
    Job(std::ifstream &jobFile);

    bool showUI() const { return m_json["showUI"].get<bool>(); }
    std::unique_ptr<Integrator> integrator() const;

private:
    nlohmann::json m_json;
};
