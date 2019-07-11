#pragma once

#include "depositer.h"
#include "point.h"
#include "transform.h"
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
    Vector3 sample(RandomGenerator &random, const Transform &worldToNormal, float *pdf, bool debug = false);
    float pdf(const Vector3 &wiWorld, const Transform &worldToNormal);

private:
    void buildCDF(const Transform &worldToNormal);

    std::vector<float> m_CDF;
    bool m_emptyCDF;

    Point3 m_origin;
    std::shared_ptr<DataSource> m_dataSource;
    std::shared_ptr<std::vector<size_t> > m_indices;
};
