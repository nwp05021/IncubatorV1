#pragma once

#include <stdint.h>

namespace incubator::domain
{
    struct AppSettings
    {
        float tempHysteresis = 0.3f;

        float humidityHysteresis = 3.0f;

        uint8_t fanNormalPwm = 40;

        uint8_t fanLockdownPwm = 60;

        uint32_t turningIntervalMs =
            7200000;

        uint32_t telemetryIntervalMs =
            60000;
    };
}