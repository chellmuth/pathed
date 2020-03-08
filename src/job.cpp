#include "job.h"

#include "albedo_integrator.h"
#include "basic_volume_integrator.h"
#include "data_parallel_integrator.h"
#include "depositer.h"
#include "light_tracer.h"
#include "ml_integrator.h"
#include "nearest_photon.h"
#include "optimal_mis_integrator.h"
#include "path_tracer.h"
#include "pdf_integrator.h"
#include "render_backsides.h"
#include "self_integrator.h"
#include "volume_path_tracer.h"

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

std::shared_ptr<Integrator> Job::integrator() const
{
    std::string integrator(m_json["integrator"].get<std::string>());

    if (integrator == "PathTracer") {
        return std::make_shared<PathTracer>(m_bounceController);
    } else if (integrator == "VolumePathTracer") {
        return std::make_shared<VolumePathTracer>(m_bounceController);
    } else if (integrator == "BasicVolumeIntegrator") {
        return std::make_shared<BasicVolumeIntegrator>(m_bounceController);
    } else if (integrator == "Depositer") {
        return std::make_shared<Depositer>(m_bounceController);
    } else if (integrator == "NearestPhoton") {
        return std::make_shared<NearestPhoton>();
    } else if (integrator == "LightTracer") {
        return std::make_shared<LightTracer>(m_bounceController);
    } else if (integrator == "PDFIntegrator") {
        return std::make_shared<PDFIntegrator>();
    } else if (integrator == "MLIntegrator") {
        return std::make_shared<MLIntegrator>(m_bounceController);
    } else if (integrator == "SelfIntegrator") {
        return std::make_shared<SelfIntegrator>(m_bounceController);
    } else if (integrator == "DataParallelIntegrator") {
        return std::make_shared<DataParallelIntegrator>(m_bounceController);
    } else if (integrator == "RenderBacksides") {
        return std::make_shared<RenderBacksides>();
    } else if (integrator == "AlbedoIntegrator") {
        return std::make_shared<AlbedoIntegrator>();
    } else if (integrator == "OptimalMISIntegrator") {
        return std::make_shared<OptimalMISIntegrator>();
    }
    throw "Unimplemented";
}
