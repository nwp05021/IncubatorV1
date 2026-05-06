#include "RuntimeScheduler.h"

#include <Arduino.h>

namespace incubator::runtime
{
    bool RuntimeScheduler::addTask(
        RuntimeTaskId id,
        RuntimePriority priority,
        const char* name,
        uint32_t intervalMs,
        uint32_t overrunLimitUs,
        TickHandler handler)
    {
        if (m_count >= MaxTasks)
        {
            return false;
        }

        if (handler == nullptr)
        {
            return false;
        }

        if (findTask(id) != nullptr)
        {
            return false;
        }

        RuntimeTask& task =
            m_tasks[m_count];

        task.id = id;
        task.priority = priority;
        task.name = name;
        task.intervalMs = intervalMs;
        task.overrunLimitUs = overrunLimitUs;
        task.lastRunMs = 0;
        task.lastExecTimeUs = 0;
        task.maxExecTimeUs = 0;
        task.runCount = 0;
        task.overrunCount = 0;
        task.handler = handler;
        task.enabled = true;
        task.overrun = false;

        ++m_count;

        return true;
    }

    void RuntimeScheduler::tick(
        uint32_t nowMs)
    {
        runPriority(
            RuntimePriority::Critical,
            nowMs);

        runPriority(
            RuntimePriority::Normal,
            nowMs);

        runPriority(
            RuntimePriority::Low,
            nowMs);
    }

    void RuntimeScheduler::runPriority(
        RuntimePriority priority,
        uint32_t nowMs)
    {
        for (uint16_t i = 0; i < m_count; ++i)
        {
            RuntimeTask& task =
                m_tasks[i];

            if (task.priority != priority)
            {
                continue;
            }

            runTask(
                task,
                nowMs);
        }
    }

    void RuntimeScheduler::runTask(
        RuntimeTask& task,
        uint32_t nowMs)
    {
        if (!task.enabled)
        {
            return;
        }

        if ((nowMs - task.lastRunMs) <
            task.intervalMs)
        {
            return;
        }

        task.lastRunMs = nowMs;

        const uint32_t startUs =
            micros();

        task.handler(nowMs);

        const uint32_t elapsedUs =
            micros() - startUs;

        task.lastExecTimeUs =
            elapsedUs;

        if (elapsedUs > task.maxExecTimeUs)
        {
            task.maxExecTimeUs =
                elapsedUs;
        }

        task.overrun =
            task.overrunLimitUs > 0 &&
            elapsedUs > task.overrunLimitUs;

        if (task.overrun)
        {
            ++task.overrunCount;
        }

        ++task.runCount;
    }

    bool RuntimeScheduler::setEnabled(
        RuntimeTaskId id,
        bool enabled)
    {
        RuntimeTask* task =
            findTaskMutable(id);

        if (task == nullptr)
        {
            return false;
        }

        task->enabled = enabled;

        return true;
    }

    const RuntimeTask* RuntimeScheduler::findTask(
        RuntimeTaskId id) const
    {
        for (uint16_t i = 0; i < m_count; ++i)
        {
            if (m_tasks[i].id == id)
            {
                return &m_tasks[i];
            }
        }

        return nullptr;
    }

    RuntimeTask* RuntimeScheduler::findTaskMutable(
        RuntimeTaskId id)
    {
        for (uint16_t i = 0; i < m_count; ++i)
        {
            if (m_tasks[i].id == id)
            {
                return &m_tasks[i];
            }
        }

        return nullptr;
    }

    uint16_t RuntimeScheduler::count() const
    {
        return m_count;
    }
}
