#pragma once

#include <random>

class RandomGenerator {
public:
    RandomGenerator();

    float next();

private:
    std::random_device m_device;
    std::mt19937 m_generator;
    std::uniform_real_distribution<> m_distribution;
};
