#pragma once

#include <stdint.h>

#include "RuntimeScheduler.h"
#include "../logging/SerialLogger.h"

namespace incubator::runtime
{
    class RuntimeSchedulerDiagnosticsBuffered
    {
    public:
        RuntimeSchedulerDiagnosticsBuffered(
            const RuntimeScheduler& scheduler,
            incubator::logging::SerialLogger& logger);

    public:
        void tick(
            uint32_t nowMs);

    private:
        void logTask(
            const RuntimeTask& task);

        const char* priorityName(
            RuntimePriority priority) const;

        void logNumber(
            const char* label,
            uint32_t value);

    private:
        const RuntimeScheduler& m_scheduler;

        incubator::logging::SerialLogger& m_logger;

        uint32_t m_lastPrintMs = 0;

        static constexpr uint32_t PrintIntervalMs =
            10000;
    };
}
