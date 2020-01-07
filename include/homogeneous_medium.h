#pragma once

#include "medium.h"

class HomogeneousMedium : public Medium {
public:
    HomogeneousMedium(Color sigmaT) : m_sigmaT(sigmaT), Medium() {}

    Color sigmaT() const override { return m_sigmaT; }

private:
    Color m_sigmaT;
};
