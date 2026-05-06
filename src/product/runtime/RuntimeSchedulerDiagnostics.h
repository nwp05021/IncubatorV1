#pragma once

#include <stdint.h>

#include "RuntimeScheduler.h"

namespace incubator::runtime
{
    class RuntimeSchedulerDiagnostics
    {
    public:
        explicit RuntimeSchedulerDiagnostics(
            const RuntimeScheduler& scheduler);

    public:
        void tick(
            uint32_t nowMs);

    private:
        void printTask(
            const RuntimeTask& task);

        const char* priorityName(
            RuntimePriority priority) const;

    private:
        const RuntimeScheduler& m_scheduler;

        uint32_t m_lastPrintMs = 0;

        static constexpr uint32_t PrintIntervalMs =
            10000;
    };
}
