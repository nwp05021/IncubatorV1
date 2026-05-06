#pragma once

#include "../../domain/RuntimeState.h"
#include "../../domain/IncubationBatch.h"
#include "../../domain/IncubationPlanTable.h"

namespace incubator::modules::scheduler
{
    class SchedulerModule
    {
    public:
        SchedulerModule(
            incubator::domain::RuntimeState& runtime,
            incubator::domain::IncubationBatch& batch,
            incubator::domain::IncubationPlanTable& plan);

    public:
        void tick(uint32_t nowMs);

    private:
        void updateCurrentDay(uint32_t nowMs);

        void applyPlan();

    private:
        incubator::domain::RuntimeState& m_runtime;

        incubator::domain::IncubationBatch& m_batch;

        incubator::domain::IncubationPlanTable& m_plan;

        uint32_t m_lastTickMs = 0;

        static constexpr uint32_t TickIntervalMs = 10000;
    };
}