#pragma once

#include "color.h"

#include <memory>

struct BSDFSample;
struct Intersection;
struct Sample;

class Medium;
class RandomGenerator;
class Scene;

namespace DirectLightingHelper {
    Color Ld(
        const Intersection &intersection,
        const std::shared_ptr<Medium> &mediumPtr,
        const BSDFSample &bsdfSample,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    );
};
