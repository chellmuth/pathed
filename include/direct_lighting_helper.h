#pragma once

#include "color.h"

struct BSDFSample;
struct Intersection;
struct Sample;

class RandomGenerator;
class Scene;

namespace DirectLightingHelper {
    Color Ld(
        const Intersection &intersection,
        const BSDFSample &bsdfSample,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    );

    Color directSampleLights(
        const Intersection &intersection,
        const BSDFSample &bsdfSample,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    );

    Color directSampleBSDF(
        const Intersection &intersection,
        const BSDFSample &bsdfSample,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    );
};
