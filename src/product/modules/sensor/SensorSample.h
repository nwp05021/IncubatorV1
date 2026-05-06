#pragma once

#include <stdint.h>

namespace incubator::modules::sensor
{
    struct SensorSample
    {
        float temperatureC = 0.0f;
        float humidityPct = 0.0f;

        bool valid = false;

        uint32_t timestampMs = 0;
    };
}