#pragma once

#include "bounce_controller.h"
#include "color.h"
#include "integrator.h"

#include "nanoflann.hpp"

#include <memory>
#include <vector>

struct DataSource
{
    struct Point
    {
        float x, y, z;
        Point3 source;
        Color throughput;
    };

    std::vector<Point> points;

    inline size_t kdtree_get_point_count() const { return points.size(); }

    inline float kdtree_get_pt(const size_t idx, const size_t dim) const
    {
        if (dim == 0) return points[idx].x;
        else if (dim == 1) return points[idx].y;
        else return points[idx].z;
    }

    template <class BBOX>
    bool kdtree_get_bbox(BBOX& /* bb */) const { return false; }
};

typedef nanoflann::KDTreeSingleIndexDynamicAdaptor<
    nanoflann::L2_Simple_Adaptor<float, DataSource>,
    DataSource,
    3
> KDTree;

class Depositer : public Integrator {
public:
    Depositer(BounceController bounceController);

    Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const override;

    void preprocess(const Scene &scene, RandomGenerator &random) override;
    void postwave(const Scene &scene, RandomGenerator &random, int waveCount) override;

    void debug(const Intersection &intersection, const Scene &scene) const override;

private:
    Color direct(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const;

    void debug2(const Intersection &intersection, const Scene &scene) const;

    BounceController m_bounceController;

    std::shared_ptr<DataSource> m_dataSource;
    KDTree *m_KDTree;

    std::shared_ptr<DataSource> m_eyeDataSource;
    KDTree *m_eyeTree;
};
