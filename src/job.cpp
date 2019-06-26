#include "job.h"

#include "depositer.h"
#include "path_tracer.h"

using json = nlohmann::json;

Job::Job(std::ifstream &jobFile)
    : m_json(json::parse(jobFile))
{}

std::unique_ptr<Integrator> Job::integrator() const
{
    std::string integrator(m_json["integrator"].get<std::string>());

    if (integrator == "PathTracer") {
        return std::make_unique<PathTracer>();
    } else if (integrator == "Depositer") {
        return std::make_unique<Depositer>();
    }
    throw "Unimplemented";
}
