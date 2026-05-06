#pragma once

#include "../../domain/RuntimeState.h"
#include "../../domain/AppSettings.h"

#include "../../devices/stepper/StepperDevice.h"

namespace incubator::modules::turning
{
    class TurningModule
    {
    public:
        TurningModule(
            incubator::domain::RuntimeState& runtime,
            incubator::domain::AppSettings& settings,
            incubator::devices::StepperDevice& stepper);

    public:
        void tick(
            uint32_t nowMs);

    private:
        incubator::domain::RuntimeState& m_runtime;

        incubator::domain::AppSettings& m_settings;

        incubator::devices::StepperDevice& m_stepper;

        uint32_t m_lastTurnMs = 0;

        bool m_forward = true;
    };
}
