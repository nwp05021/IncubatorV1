#include "FanModule.h"

namespace incubator::modules::fan
{
    using namespace incubator::domain;

    FanModule::FanModule(
        RuntimeState& runtime,
        AppSettings& settings,
        incubator::devices::PwmFanDevice& fan)
        :
        m_runtime(runtime),
        m_settings(settings),
        m_fan(fan)
    {
    }

    void FanModule::tick(
        uint32_t nowMs)
    {
        if ((nowMs - m_lastTickMs) <
            TickIntervalMs)
        {
            return;
        }

        m_lastTickMs = nowMs;

        applyFanPolicy();
    }

    void FanModule::applyFanPolicy()
    {
        if (m_runtime.safeMode)
        {
            m_runtime.fanPwm = 0;

            m_fan.setDuty(0);

            return;
        }

        uint8_t pwm =
            m_settings.fanNormalPwm;

        if (m_runtime.lockdown)
        {
            pwm =
                m_settings.fanLockdownPwm;
        }

        if (m_runtime.currentTempC >
            m_runtime.targetTempC + 0.5f)
        {
            if (pwm <= 90)
            {
                pwm += 10;
            }
            else
            {
                pwm = 100;
            }
        }

        m_runtime.fanPwm = pwm;

        m_fan.setDuty(pwm);
    }
}
