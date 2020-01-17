#pragma once

#include "color.h"

class Medium;
class Point3;
class RandomGenerator;
class Scene;

namespace VolumeHelper {
    Color directSampleLights(
        const Medium &medium,
        const Point3 &samplePoint,
        const Scene &scene,
        RandomGenerator &random
    );
};
