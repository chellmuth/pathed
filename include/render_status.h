#pragma once

class RenderStatus {
public:
    void setSample(int sample) { m_sample = sample; }
    int sample() const { return m_sample; }

private:
    int m_sample;
};
