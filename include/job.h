#pragma once

#include "json.hpp"

#include <iostream>
#include <fstream>

class Job {
public:
    Job(std::ifstream &jobFile);

    bool showUI() const { return m_json["showUI"].get<bool>(); }

private:
    nlohmann::json m_json;
};
