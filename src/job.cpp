#include "job.h"

using json = nlohmann::json;

Job::Job(std::ifstream &jobFile)
    : m_json(json::parse(jobFile))
{}
