#include "RuntimeSchedulerDiagnosticsBuffered.h"

#include <stdio.h>

namespace incubator::runtime
{
    RuntimeSchedulerDiagnosticsBuffered::RuntimeSchedulerDiagnosticsBuffered(
        const RuntimeScheduler& scheduler,
        incubator::logging::SerialLogger& logger)
        :
        m_scheduler(scheduler),
        m_logger(logger)
    {
    }

    void RuntimeSchedulerDiagnosticsBuffered::tick(
        uint32_t nowMs)
    {
        if ((nowMs - m_lastPrintMs) <
            PrintIntervalMs)
        {
            return;
        }

        m_lastPrintMs = nowMs;

        m_logger.writeLine("");
        m_logger.writeLine("[DIAG] RuntimeScheduler");

        const RuntimeTaskId ids[] =
        {
            RuntimeTaskId::Time,
            RuntimeTaskId::Performance,
            RuntimeTaskId::Sensor,
            RuntimeTaskId::Scheduler,
            RuntimeTaskId::Climate,
            RuntimeTaskId::Turning,
            RuntimeTaskId::Fan,
            RuntimeTaskId::Alarm,
            RuntimeTaskId::Recovery,
            RuntimeTaskId::AppController,
            RuntimeTaskId::Event,
            RuntimeTaskId::Toast,
            RuntimeTaskId::UI,
            RuntimeTaskId::Wifi,
            RuntimeTaskId::Aws,
            RuntimeTaskId::Shadow,
            RuntimeTaskId::Diagnostics,
            RuntimeTaskId::SchedulerDiagnostics
        };

        for (const RuntimeTaskId id : ids)
        {
            const RuntimeTask* task =
                m_scheduler.findTask(id);

            if (task != nullptr)
            {
                logTask(*task);
            }
        }
    }

    void RuntimeSchedulerDiagnosticsBuffered::logTask(
        const RuntimeTask& task)
    {
        char line[160];

        snprintf(
            line,
            sizeof(line),
            "%s | %s run=%lu lastUs=%lu maxUs=%lu overrun=%lu enabled=%s",
            priorityName(task.priority),
            task.name,
            static_cast<unsigned long>(task.runCount),
            static_cast<unsigned long>(task.lastExecTimeUs),
            static_cast<unsigned long>(task.maxExecTimeUs),
            static_cast<unsigned long>(task.overrunCount),
            task.enabled ? "Y" : "N");

        m_logger.writeLine(line);
    }

    const char* RuntimeSchedulerDiagnosticsBuffered::priorityName(
        RuntimePriority priority) const
    {
        switch (priority)
        {
            case RuntimePriority::Critical:
                return "CRIT";

            case RuntimePriority::Normal:
                return "NORM";

            case RuntimePriority::Low:
                return "LOW";

            default:
                return "?";
        }
    }
}
