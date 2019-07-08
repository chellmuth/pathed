#include "photon_visualization.h"

#include "json.hpp"
using json = nlohmann::json;

#include <fstream>
#include <iostream>

void PhotonVisualization::all(const Intersection &intersection, const DataSource &dataSource)
{
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

    std::ofstream jsonFile("live-photons.json");
    jsonFile << j.dump(4) << std::endl;
    std::cout << "Wrote to live-photons.json" << std::endl;
}
