#pragma once

#include "../../domain/RuntimeState.h"
#include "../../domain/AppSettings.h"

#include "../../devices/output/RelayDevice.h"

namespace incubator::modules::climate
{
    class ClimateModule
    {
    public:
        ClimateModule(
            incubator::domain::RuntimeState& runtime,
            incubator::domain::AppSettings& settings,
            incubator::devices::RelayDevice& heater,
            incubator::devices::RelayDevice& humidifier);

    public:
        void tick(uint32_t nowMs);

    private:
        incubator::domain::RuntimeState& m_runtime;

        incubator::domain::AppSettings& m_settings;

        incubator::devices::RelayDevice& m_heater;

        incubator::devices::RelayDevice& m_humidifier;

        uint32_t m_lastTickMs = 0;

        static constexpr uint32_t TickIntervalMs =
            500;
    };
}
