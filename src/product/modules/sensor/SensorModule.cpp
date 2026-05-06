#include "SensorModule.h"

namespace incubator::modules::sensor
{
    using namespace incubator::domain;

    SensorModule::SensorModule(
        RuntimeState& runtime,
        incubator::devices::Aht20Device& device)
        :
        m_runtime(runtime),
        m_device(device)
    {
    }

    void SensorModule::tick(
        uint32_t nowMs)
    {
        if ((nowMs - m_lastPollMs) <
            PollIntervalMs)
        {
            return;
        }

        m_lastPollMs = nowMs;

        float tempC = 0.0f;

        float humidityPct = 0.0f;

        if (!m_device.read(
            tempC,
            humidityPct))
        {
            ++m_failCount;

            if (m_failCount >= 3)
            {
                m_runtime.sensorHealthy = false;
            }

            return;
        }

        m_failCount = 0;

        m_runtime.sensorHealthy = true;

        m_runtime.currentTempC = tempC;

        m_runtime.currentHumidityPct =
            humidityPct;
    }
}
