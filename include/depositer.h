#pragma once

#include "integrator.h"

#include "nanoflann.hpp"

#include <vector>

struct DataSource
{
	struct Point
	{
		float x, y, z;
	};

	std::vector<Point>  pts;

	inline size_t kdtree_get_point_count() const { return pts.size(); }

	inline float kdtree_get_pt(const size_t idx, const size_t dim) const
	{
		if (dim == 0) return pts[idx].x;
		else if (dim == 1) return pts[idx].y;
		else return pts[idx].z;
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
    Color L(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        int bounceCount,
        Sample &sample
    ) const override;

    void preprocess(const Scene &scene, RandomGenerator &random) override;

private:
    Color direct(
        const Intersection &intersection,
        const Scene &scene,
        RandomGenerator &random,
        Sample &sample
    ) const;

    DataSource mDataSource;
    KDTree *mKDTree;
};
