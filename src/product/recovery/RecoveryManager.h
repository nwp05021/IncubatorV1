#pragma once

#include "../domain/RuntimeState.h"
#include "../domain/AlarmState.h"

namespace incubator::recovery
{
    class RecoveryManager
    {
    public:
        RecoveryManager(
            incubator::domain::RuntimeState& runtime,
            incubator::domain::AlarmState& alarm);

    public:
        void tick();

    private:
        incubator::domain::RuntimeState& m_runtime;

        incubator::domain::AlarmState& m_alarm;
    };
}
