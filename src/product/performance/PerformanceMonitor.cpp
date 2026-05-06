#include "PerformanceMonitor.h"
#include <Arduino.h>

namespace incubator::performance
{
    PerformanceMonitor::PerformanceMonitor(PerformanceState& state)
        : m_state(state)
    {
    }

    void PerformanceMonitor::beginLoop()
    {
        m_loopStartUs = micros();
    }

    void PerformanceMonitor::endLoop()
    {
        const uint32_t elapsed = micros() - m_loopStartUs;

        m_state.lastLoopTimeUs = elapsed;

        if (elapsed > m_state.maxLoopTimeUs)
        {
            m_state.maxLoopTimeUs = elapsed;
        }

        m_state.loopOverrun = elapsed > LoopOverrunUs;

        ++m_state.loopCount;
    }

    void PerformanceMonitor::tick(uint32_t nowMs)
    {
        if ((nowMs - m_lastTickMs) < TickIntervalMs)
        {
            return;
        }

        m_lastTickMs = nowMs;

        m_state.freeHeap = ESP.getFreeHeap();

        if (m_state.minFreeHeap == 0 ||
            m_state.freeHeap < m_state.minFreeHeap)
        {
            m_state.minFreeHeap = m_state.freeHeap;
        }
    }
}
