#include "TimeService.h"

namespace incubator::time
{
    void TimeService::begin()
    {
        m_epoch = 0;
    }

    void TimeService::tick(
        uint32_t nowMs)
    {
        if ((nowMs - m_lastTickMs) >=
            1000)
        {
            ++m_epoch;

            m_lastTickMs = nowMs;
        }
    }

    uint32_t TimeService::epoch() const
    {
        return m_epoch;
    }
}
