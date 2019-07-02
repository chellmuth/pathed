#include "bounce_controller.h"

#include <algorithm>
#include <assert.h>

BounceController::BounceController(int startBounce, int lastBounce)
    : m_startBounce(startBounce),
      m_lastBounce(lastBounce)
{
    assert(m_startBounce >= 0);
    assert(m_lastBounce == -1 || m_startBounce <= m_lastBounce);
}

bool BounceController::checkCounts(int bounce) const
{
    if (m_startBounce > bounce) { return false; }
    return !checkDone(bounce);
}

bool BounceController::checkDone(int bounce) const
{
    if (m_lastBounce == -1) { return false; }

    return bounce > m_lastBounce;
}

BounceController BounceController::copyAfterBounce() const
{
    int startBounce = std::max(0, m_startBounce - 1);
    int lastBounce;
    if (m_lastBounce == -1) {
        lastBounce = -1;
    } else {
        lastBounce = std::max(0, m_lastBounce - 1);
    }

    return BounceController(startBounce, lastBounce);
}
