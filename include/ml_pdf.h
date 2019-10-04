#pragma once

#include <vector>

class MLPDF {
public:
    MLPDF();

    bool connectToModel();
    void sample(float *x, float *y, float *pdf, std::vector<float> &photonBundle) const;
    void estimatePDF(std::vector<float> &radianceLookup, std::vector<float> &photonBundle) const;

private:
    int m_socket;
};
