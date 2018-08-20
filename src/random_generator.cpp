#include "random_generator.h"

// FIXME: Not sure why, but m_distribution was returning an inclusive 1
RandomGenerator::RandomGenerator()
    : m_generator(m_device()), m_distribution(0.f, 1.f - std::numeric_limits<float>::epsilon())
{}

float RandomGenerator::next()
{
    return m_distribution(m_generator);
}
