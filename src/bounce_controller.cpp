#include "bounce_controller.h"

#include <assert.h>

BounceController::BounceController(int startBounce, int lastBounce)
    : m_startBounce(startBounce),
      m_lastBounce(lastBounce)
{
    assert(m_startBounce >= 0);
    assert(m_lastBounce == -1 || m_startBounce <= m_endBounce);
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
