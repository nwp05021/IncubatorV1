#pragma once

#include "../../domain/RuntimeState.h"
#include "../../domain/AppSettings.h"

#include "../../devices/fan/PwmFanDevice.h"

namespace incubator::modules::fan
{
    class FanModule
    {
    public:
        FanModule(
            incubator::domain::RuntimeState& runtime,
            incubator::domain::AppSettings& settings,
            incubator::devices::PwmFanDevice& fan);

    public:
        void tick(
            uint32_t nowMs);

    private:
        void applyFanPolicy();

    private:
        incubator::domain::RuntimeState& m_runtime;

        incubator::domain::AppSettings& m_settings;

        incubator::devices::PwmFanDevice& m_fan;

        uint32_t m_lastTickMs = 0;

        static constexpr uint32_t TickIntervalMs =
            1000;
    };
}
