#pragma once

#include <stdint.h>

namespace incubator::runtime
{
    enum class RuntimeTaskId
    {
        Time,
        Diagnostics,
        SchedulerDiagnostics,
        Sensor,
        Scheduler,
        Climate,
        Turning,
        Fan,
        Alarm,
        Recovery,
        AppController,
        Event,
        Toast,
        UI,
        Wifi,
        Aws,
        Shadow,
        Performance
    };

    enum class RuntimePriority
    {
        Critical = 0,
        Normal = 1,
        Low = 2
    };

    using TickHandler =
        void (*)(uint32_t nowMs);

    struct RuntimeTask
    {
        RuntimeTaskId id;

        RuntimePriority priority =
            RuntimePriority::Normal;

        const char* name = "";

        uint32_t intervalMs = 0;

        uint32_t lastRunMs = 0;

        uint32_t lastExecTimeUs = 0;

        uint32_t maxExecTimeUs = 0;

        uint32_t overrunLimitUs = 0;

        uint32_t runCount = 0;

        uint32_t overrunCount = 0;

        TickHandler handler = nullptr;

        bool enabled = true;

        bool overrun = false;
    };
}
