#pragma once

#include "kd_tree.h"
#include "sample.h"

#include <memory>
#include <vector>

class RenderStatus {
public:
    void setSample(int sample) { m_sample = sample; }
    int sample() const { return m_sample; }

    std::shared_ptr<std::vector<Sample> > sampleLookup() const { return m_sampleLookup; }
    void setSampleLookup(std::shared_ptr<std::vector<Sample> > sampleLookup) {
        m_sampleLookup = sampleLookup;
    }

    std::shared_ptr<std::vector<DataSource::Point> > photons() const { return m_photons; }
    void setPhotons(std::shared_ptr<std::vector<DataSource::Point> > photons) {
        m_photons = photons;
    }

    std::shared_ptr<DataSource> dataSource() const { return m_dataSource; }
    void setDataSource(std::shared_ptr<DataSource> dataSource) {
        m_dataSource = dataSource;
    }

private:
    int m_sample;
    std::shared_ptr<std::vector<Sample> > m_sampleLookup;
    std::shared_ptr<std::vector<DataSource::Point> > m_photons;
    std::shared_ptr<DataSource> m_dataSource;
};
