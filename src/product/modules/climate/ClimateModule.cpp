#include "ClimateModule.h"

namespace incubator::modules::climate
{
    using namespace incubator::domain;

    ClimateModule::ClimateModule(
        RuntimeState& runtime,
        AppSettings& settings,
        incubator::devices::RelayDevice& heater,
        incubator::devices::RelayDevice& humidifier)
        :
        m_runtime(runtime),
        m_settings(settings),
        m_heater(heater),
        m_humidifier(humidifier)
    {
    }

    void ClimateModule::tick(
        uint32_t nowMs)
    {
        if ((nowMs - m_lastTickMs) <
            TickIntervalMs)
        {
            return;
        }

        m_lastTickMs = nowMs;

        if (m_runtime.safeMode)
        {
            m_heater.off();
            m_humidifier.off();

            return;
        }

        const float tempLow =
            m_runtime.targetTempC -
            m_settings.tempHysteresis;

        const float tempHigh =
            m_runtime.targetTempC +
            m_settings.tempHysteresis;

        if (m_runtime.currentTempC < tempLow)
        {
            m_heater.on();

            m_runtime.heaterOn = true;
        }
        else if (m_runtime.currentTempC > tempHigh)
        {
            m_heater.off();

            m_runtime.heaterOn = false;
        }

        const float humidityLow =
            m_runtime.targetHumidityPct -
            m_settings.humidityHysteresis;

        const float humidityHigh =
            m_runtime.targetHumidityPct +
            m_settings.humidityHysteresis;

        if (m_runtime.currentHumidityPct < humidityLow)
        {
            m_humidifier.on();

            m_runtime.humidifierOn = true;
        }
        else if (m_runtime.currentHumidityPct > humidityHigh)
        {
            m_humidifier.off();

            m_runtime.humidifierOn = false;
        }
    }
}
