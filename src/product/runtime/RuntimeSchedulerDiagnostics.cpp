#include "RuntimeSchedulerDiagnostics.h"

#include <Arduino.h>

namespace incubator::runtime
{
    RuntimeSchedulerDiagnostics::RuntimeSchedulerDiagnostics(
        const RuntimeScheduler& scheduler)
        :
        m_scheduler(scheduler)
    {
    }

    void RuntimeSchedulerDiagnostics::tick(
        uint32_t nowMs)
    {
        if ((nowMs - m_lastPrintMs) <
            PrintIntervalMs)
        {
            return;
        }

        m_lastPrintMs = nowMs;

        Serial.println();
        Serial.println("[DIAG] RuntimeScheduler");

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
                printTask(*task);
            }
        }
    }

    void RuntimeSchedulerDiagnostics::printTask(
        const RuntimeTask& task)
    {
        Serial.print(priorityName(task.priority));
        Serial.print(" | ");
        Serial.print(task.name);
        Serial.print(" run=");
        Serial.print(task.runCount);
        Serial.print(" lastUs=");
        Serial.print(task.lastExecTimeUs);
        Serial.print(" maxUs=");
        Serial.print(task.maxExecTimeUs);
        Serial.print(" overrun=");
        Serial.print(task.overrunCount);
        Serial.print(" enabled=");
        Serial.println(task.enabled ? "Y" : "N");
    }

    const char* RuntimeSchedulerDiagnostics::priorityName(
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
