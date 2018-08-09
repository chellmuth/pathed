#include "random_generator.h"

RandomGenerator::RandomGenerator()
    : m_generator(m_device()), m_distribution(0.0, 1.0)
{}

float RandomGenerator::next()
{
    return m_distribution(m_generator);
}
