#pragma once

#include "color.h"

class Medium {
public:
    Medium(Color sigmaT) : m_sigmaT(sigmaT) {}

    Color sigmaT() const { return m_sigmaT; }

private:
    Color m_sigmaT;
};
