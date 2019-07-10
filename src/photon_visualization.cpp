#include "photon_visualization.h"

#include "globals.h"
#include "job.h"

#include "json.hpp"
using json = nlohmann::json;

#include <fstream>
#include <iostream>
#include <sstream>

void PhotonVisualization::all(
    const Intersection &intersection,
    const DataSource &dataSource,
    int waveCount
) {
    Point3 intersectionPoint = intersection.point;

    json j;
    j["QueryPoint"] = { intersectionPoint.x(), intersectionPoint.y(), intersectionPoint.z() };
    j["Results"] = json::array();
    for (auto &point : dataSource.points) {
        j["Results"].push_back({
            { "point", { point.x, point.y, point.z } },
            { "source", { point.source.x(), point.source.y(), point.source.z() } },
            { "throughput", { point.throughput.r(), point.throughput.g(), point.throughput.b() } },
        });
    }

    std::ostringstream filenameStream;
    filenameStream << "live-photons-" << waveCount << ".json";

    std::string jsonPath = g_job->visualizationPath(filenameStream.str());
    std::ofstream jsonFile(jsonPath);
    jsonFile << j.dump(4) << std::endl;
    std::cout << "Wrote to live-photons.json" << std::endl;
}
