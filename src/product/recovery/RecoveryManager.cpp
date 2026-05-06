#include "RecoveryManager.h"

namespace incubator::recovery
{
    using namespace incubator::domain;

    RecoveryManager::RecoveryManager(
        RuntimeState& runtime,
        AlarmState& alarm)
        :
        m_runtime(runtime),
        m_alarm(alarm)
    {
    }

    void RecoveryManager::tick()
    {
        if (m_alarm.sensorFail)
        {
            m_runtime.safeMode = true;

            m_runtime.heaterOn = false;

            m_runtime.humidifierOn = false;

            m_runtime.turnerOn = false;

            m_runtime.fanPwm = 0;
        }
    }
}
