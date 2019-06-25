#pragma once

#include "depositer.h"
#include "point.h"
#include "vector.h"

#include <memory>
#include <vector>

class PhotonPDF {
public:
    PhotonPDF(
        const Point3 &origin,
        std::shared_ptr<DataSource> dataSource,
        std::shared_ptr<std::vector<size_t> > indices
    );
    Vector3 sample(RandomGenerator &random, float *pdf);

private:
    Point3 mOrigin;
    std::shared_ptr<DataSource> mDataSource;
    std::shared_ptr<std::vector<size_t> > mIndices;
};
