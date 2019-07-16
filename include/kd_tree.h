#pragma once

#include "color.h"
#include "point.h"

#include "nanoflann.hpp"

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

typedef nanoflann::KDTreeSingleIndexAdaptor<
    nanoflann::L2_Simple_Adaptor<float, DataSource>,
    DataSource,
    3
> KDTree;
