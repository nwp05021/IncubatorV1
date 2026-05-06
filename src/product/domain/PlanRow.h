#pragma once

#include <stdint.h>

namespace incubator::domain
{
    struct PlanRow
    {
        uint16_t day = 0;

        float targetTempC = 37.5f;

        float targetHumidityPct = 60.0f;

        bool turningEnabled = true;

        uint32_t turningIntervalMs = 7200000;
    };
}