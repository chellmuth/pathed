#pragma once

#include "depositer.h"
#include "intersection.h"

#include <string>

namespace PhotonVisualization {
    void all(
        const Intersection &intersection,
        const DataSource &dataSource,
        const std::string &filename,
        int waveCount = 0
    );
};
