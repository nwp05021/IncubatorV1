#pragma once

#include "../../domain/RuntimeState.h"
#include "../../domain/AlarmState.h"

namespace incubator::modules::alarm
{
    class AlarmModule
    {
    public:
        AlarmModule(
            incubator::domain::RuntimeState& runtime,
            incubator::domain::AlarmState& alarm);

    public:
        void tick(uint32_t nowMs);

    private:
        incubator::domain::RuntimeState& m_runtime;

        incubator::domain::AlarmState& m_alarm;

        uint32_t m_highTempStartMs = 0;

        static constexpr uint32_t AlarmDelayMs =
            10000;
    };
}
