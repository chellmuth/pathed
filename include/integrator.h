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

    virtual Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const = 0;

    virtual void preprocess(const Scene &scene, RandomGenerator &random) {};
    virtual void postwave(const Scene &scene, RandomGenerator &random, int waveCount) {};
    virtual void debug(const Intersection &intersection, const Scene &scene) const {};

protected:
    //temp!! for real
    virtual std::vector<DataSource::Point> getPhotons() const {
        std::vector<DataSource::Point> dummy;
        DataSource::Point point = {
            .x = 4.f,
            .y = 1.f,
            .z = -2.f,
            .source = Point3(0.f, 0.f, 0.f),
            .throughput = Color(1.f)
        };
        dummy.push_back(point);
        return dummy;
    };

private:
    void sampleImage(
        std::vector<float> &radianceLookup,
        std::vector<Sample> &sampleLookup,
        Scene &scene,
        RandomGenerator &random
    );

    void samplePixel(
        int row, int col,
        int width, int height,
        std::vector<float> &radianceLookup,
        std::vector<Sample> &sampleLookup,
        const Scene &scene,
        RandomGenerator &random
    );

};
