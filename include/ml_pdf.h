#pragma once

#include <vector>

class MLPDF {
public:
    MLPDF();

    bool connectToModel(int portOffset);
    void sample(float *x, float *y, float *pdf, std::vector<float> &photonBundle) const;
    void batchSample(
        int count,
        std::vector<float> &phis,
        std::vector<float> &thetas,
        std::vector<float> &pdfs,
        std::vector<float> &photonBundles
    ) const;

    void batchEval(
        int count,
        std::vector<float> &pdfs,
        std::vector<float> &photonBundles
    ) const;

    void estimatePDF(std::vector<float> &radianceLookup, std::vector<float> &photonBundle) const;

private:
    int m_socket;
};

class MLPDFPool {
public:
    MLPDFPool();

    bool connectToModel();
    void sample(float *x, float *y, float *pdf, std::vector<float> &photonBundle) const;

private:
    std::vector<MLPDF> m_pdfs;
};
