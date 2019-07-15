#pragma once

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

private:
    int m_sample;
    std::shared_ptr<std::vector<Sample> > m_sampleLookup;
};
