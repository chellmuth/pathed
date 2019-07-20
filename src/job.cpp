#include "job.h"

#include "depositer.h"
#include "light_tracer.h"
#include "nearest_photon.h"
#include "path_tracer.h"

#include <errno.h>
#include <iomanip>
#include <sstream>
#include <stdlib.h>
#include <sys/stat.h>

using json = nlohmann::json;

Job::Job(std::ifstream &jobFile)
    : m_json(json::parse(jobFile)),
      m_bounceController(
          m_json["startBounce"].get<int>(),
          m_json["lastBounce"].get<int>()
      )
{}

void Job::init()
{
    std::string directory = outputDirectory();

    int result = mkdir(directory.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (result == -1) {
        if (errno == EEXIST) {
            std::cout << "Output directory already exists: " << directory << std::endl;
        } else {
            std::cout << "Failed to create: " << directory << std::endl;
        }
        if (!force()) {
            exit(1);
        }
    }

    result = mkdir(visualizationDirectory().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (result == -1) {
        std::cout << "Failed to create: " << visualizationDirectory() << std::endl;
        if (!force()) {
            exit(1);
        }
    }

    std::ostringstream outputFilenameStream;
    outputFilenameStream << directory << "/" << "report.json";
    std::string outputFilename = outputFilenameStream.str();

    std::ofstream outputStream(outputFilename);
    outputStream << std::setw(4) << m_json << std::endl;
}

std::unique_ptr<Integrator> Job::integrator() const
{
    std::string integrator(m_json["integrator"].get<std::string>());

    if (integrator == "PathTracer") {
        return std::make_unique<PathTracer>(m_bounceController);
    } else if (integrator == "Depositer") {
        return std::make_unique<Depositer>(m_bounceController);
    } else if (integrator == "NearestPhoton") {
        return std::make_unique<NearestPhoton>();
    } else if (integrator == "LightTracer") {
        return std::make_unique<LightTracer>(m_bounceController);
    }
    throw "Unimplemented";
}
