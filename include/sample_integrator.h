#pragma once

#include "integrator.h"
#include "random_generator.h"
#include "sample.h"
#include "scene.h"

#include <vector>

class SampleIntegrator : public Integrator {
public:

protected:
    void sampleImage(
        std::vector<float> &radianceLookup,
        std::vector<Sample> &sampleLookup,
        Scene &scene,
        RandomGenerator &random
    ) override;

    void samplePixel(
        int row, int col,
        int width, int height,
        std::vector<float> &radianceLookup,
        std::vector<Sample> &sampleLookup,
        const Scene &scene,
        RandomGenerator &random
    );

};
