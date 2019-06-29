#pragma once

class BounceController {
public:
    BounceController(int startBounce, int lastBounce);

    bool checkCounts(int bounce) const;
    bool checkDone(int bounce) const;

    int startBounce() const { return m_startBounce; }
    int lastBounce() const { return m_lastBounce; }

private:
    int m_startBounce;
    int m_lastBounce;
};
