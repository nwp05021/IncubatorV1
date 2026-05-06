#include "SchedulerModule.h"

namespace incubator::modules::scheduler
{
    using namespace incubator::domain;

    SchedulerModule::SchedulerModule(
        RuntimeState& runtime,
        IncubationBatch& batch,
        IncubationPlanTable& plan)
        :
        m_runtime(runtime),
        m_batch(batch),
        m_plan(plan)
    {
    }

    void SchedulerModule::tick(uint32_t nowMs)
    {
        if ((nowMs - m_lastTickMs) < TickIntervalMs)
        {
            return;
        }

        m_lastTickMs = nowMs;

        if (!m_batch.active)
        {
            return;
        }

        updateCurrentDay(nowMs);

        applyPlan();
    }

    void SchedulerModule::updateCurrentDay(uint32_t nowMs)
    {
        const uint32_t elapsedSec =
            (nowMs / 1000);

        const uint32_t elapsedDays =
            elapsedSec / 86400;

        m_runtime.currentDay =
            static_cast<uint16_t>(elapsedDays + 1);

        m_runtime.totalDays =
            m_batch.totalDays;

        m_runtime.lockdown =
            (m_runtime.currentDay >=
             m_batch.lockdownStartDay);
    }

    void SchedulerModule::applyPlan()
    {
        PlanRow row;

        if (!m_plan.getRow(
                m_runtime.currentDay,
                row))
        {
            return;
        }

        m_runtime.targetTempC =
            row.targetTempC;

        m_runtime.targetHumidityPct =
            row.targetHumidityPct;
    }
}