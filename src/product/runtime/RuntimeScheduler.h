#pragma once

#include <stdint.h>

#include "RuntimeTask.h"

namespace incubator::runtime
{
    class RuntimeScheduler
    {
    public:
        static constexpr uint16_t MaxTasks = 24;

    public:
        bool addTask(
            RuntimeTaskId id,
            RuntimePriority priority,
            const char* name,
            uint32_t intervalMs,
            uint32_t overrunLimitUs,
            TickHandler handler);

        void tick(
            uint32_t nowMs);

        bool setEnabled(
            RuntimeTaskId id,
            bool enabled);

        const RuntimeTask* findTask(
            RuntimeTaskId id) const;

        uint16_t count() const;

    private:
        void runPriority(
            RuntimePriority priority,
            uint32_t nowMs);

        void runTask(
            RuntimeTask& task,
            uint32_t nowMs);

        RuntimeTask* findTaskMutable(
            RuntimeTaskId id);

    private:
        RuntimeTask m_tasks[MaxTasks];

        uint16_t m_count = 0;
    };
}
