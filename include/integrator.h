#pragma once

#include "color.h"
#include "image.h"
#include "intersection.h"
#include "random_generator.h"
#include "scene.h"
#include "point.h"
#include "render_status.h"

#include <functional>
#include <vector>

struct Sample {
    std::vector<Point3> eyePoints;
    std::vector<Point3> shadowPoints;

    Sample()
    : eyePoints(), shadowPoints()
    {}
};

class Integrator {
public:
    void run(
        Image &image,
        Scene &scene,
        std::function<void(RenderStatus)> callback,
        bool *quit
    );

    virtual Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const = 0;

    virtual void preprocess(const Scene &scene, RandomGenerator &random) {};
    virtual void postwave(const Scene &scene, RandomGenerator &random, int waveCount) {};
    virtual void debug(const Intersection &intersection, const Scene &scene) const {};

private:
    void sampleImage(
        std::vector<float> &radianceLookup,
        Scene &scene,
        RandomGenerator &random
    );

    void samplePixel(
        int row, int col,
        int width, int height,
        std::vector<float> &radianceLookup,
        const Scene &scene,
        RandomGenerator &random
    );

};
