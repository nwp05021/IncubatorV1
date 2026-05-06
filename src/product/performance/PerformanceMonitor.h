#pragma once

#include "PerformanceState.h"

namespace incubator::performance
{
    class PerformanceMonitor
    {
    public:
        PerformanceMonitor(
            PerformanceState& state);

    public:
        void beginLoop();

        void endLoop();

        void tick(uint32_t nowMs);

    private:
        PerformanceState& m_state;

        uint32_t m_loopStartUs = 0;

        uint32_t m_lastTickMs = 0;

        static constexpr uint32_t TickIntervalMs =
            5000;

        static constexpr uint32_t LoopOverrunUs =
            20000;
    };
}