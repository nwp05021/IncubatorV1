#include "TurningModule.h"

namespace incubator::modules::turning
{
    using namespace incubator::domain;

    TurningModule::TurningModule(
        RuntimeState& runtime,
        AppSettings& settings,
        incubator::devices::StepperDevice& stepper)
        :
        m_runtime(runtime),
        m_settings(settings),
        m_stepper(stepper)
    {
    }

    void TurningModule::tick(
        uint32_t nowMs)
    {
        if (m_runtime.safeMode ||
            m_runtime.lockdown)
        {
            m_stepper.stop();

            m_runtime.turnerOn = false;

            return;
        }

        if ((nowMs - m_lastTurnMs) <
            m_settings.turningIntervalMs)
        {
            return;
        }

        m_lastTurnMs = nowMs;

        if (m_forward)
        {
            m_stepper.stepForward();
        }
        else
        {
            m_stepper.stepBackward();
        }

        m_forward = !m_forward;

        m_runtime.turnerOn = true;
    }
}
