#pragma once

#include "color.h"

#include <memory>
#include <vector>

class Medium;
class Point3;
class RandomGenerator;
class Ray;
class Scene;

struct VolumeEvent;

namespace VolumeHelper {
    Color directSampleLights(
        const Medium &medium,
        const Point3 &samplePoint,
        const Scene &scene,
        RandomGenerator &random
    );

    Color rayTransmission(
        const Ray &ray,
        const std::vector<VolumeEvent> &volumeEvents,
        const std::shared_ptr<Medium> &mediumPtr
    );

};
