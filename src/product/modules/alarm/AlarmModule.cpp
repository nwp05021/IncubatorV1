#include "AlarmModule.h"

namespace incubator::modules::alarm
{
    using namespace incubator::domain;

    AlarmModule::AlarmModule(
        RuntimeState& runtime,
        AlarmState& alarm)
        :
        m_runtime(runtime),
        m_alarm(alarm)
    {
    }

    void AlarmModule::tick(
        uint32_t nowMs)
    {
        const float highLimit =
            m_runtime.targetTempC + 1.5f;

        const float lowLimit =
            m_runtime.targetTempC - 2.0f;

        if (m_runtime.currentTempC >
            highLimit)
        {
            if (m_highTempStartMs == 0)
            {
                m_highTempStartMs = nowMs;
            }

            if ((nowMs - m_highTempStartMs) >=
                AlarmDelayMs)
            {
                m_alarm.highTemp = true;
            }
        }
        else
        {
            m_alarm.highTemp = false;

            m_highTempStartMs = 0;
        }

        m_alarm.lowTemp =
            m_runtime.currentTempC < lowLimit;

        m_alarm.sensorFail =
            !m_runtime.sensorHealthy;
    }
}
