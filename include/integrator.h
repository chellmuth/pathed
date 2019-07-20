#pragma once

#include "color.h"
#include "image.h"
#include "intersection.h"
#include "kd_tree.h"
#include "random_generator.h"
#include "sample.h"
#include "scene.h"
#include "point.h"
#include "render_status.h"

#include <functional>
#include <vector>

class Integrator {
public:
    void run(
        Image &image,
        Scene &scene,
        std::function<void(RenderStatus)> callback,
        bool *quit
    );

    virtual void preprocess(const Scene &scene, RandomGenerator &random) {};
    virtual void postwave(const Scene &scene, RandomGenerator &random, int waveCount) {};
    virtual void debug(const Intersection &intersection, const Scene &scene) const {};

protected:
    //temp!! for real
    virtual std::vector<DataSource::Point> getPhotons() const {
        std::vector<DataSource::Point> dummy;
        return dummy;
    };

    virtual DataSource getDataSource() const {
        DataSource dummy;
        return dummy;
    };

    virtual void sampleImage(
        std::vector<float> &radianceLookup,
        std::vector<Sample> &sampleLookup,
        Scene &scene,
        RandomGenerator &random
    ) = 0;
};
