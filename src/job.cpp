#include "job.h"

#include "depositer.h"
#include "path_tracer.h"

#include <iomanip>
#include <sstream>
#include <stdlib.h>
#include <sys/stat.h>

using json = nlohmann::json;

Job::Job(std::ifstream &jobFile)
    : m_json(json::parse(jobFile))
{}

void Job::init()
{
    std::string directory = outputDirectory();

    mkdir(directory.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    json jsonReport;
    jsonReport["name"] = outputName();

    std::ostringstream outputFilenameStream;
    outputFilenameStream << directory << "/" << "report.json";
    std::string outputFilename = outputFilenameStream.str();

    std::ofstream outputStream(outputFilename);
    outputStream << std::setw(4) << jsonReport << std::endl;
}

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
