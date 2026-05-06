#pragma once

#include "../../domain/RuntimeState.h"

#include "../../devices/sensor/Aht20Device.h"

namespace incubator::modules::sensor
{
    class SensorModule
    {
    public:
        SensorModule(
            incubator::domain::RuntimeState& runtime,
            incubator::devices::Aht20Device& device);

    public:
        void tick(uint32_t nowMs);

    private:
        incubator::domain::RuntimeState& m_runtime;

        incubator::devices::Aht20Device& m_device;

        uint32_t m_lastPollMs = 0;

        uint32_t m_failCount = 0;

        static constexpr uint32_t PollIntervalMs =
            2000;
    };
}
