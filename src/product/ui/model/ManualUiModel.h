#pragma once

#include <stdint.h>

namespace incubator::ui
{
    struct ManualUiModel
    {
        bool heaterOn = false;

        bool humidifierOn = false;

        bool turnerOn = false;

        uint8_t fanPwm = 0;

        bool safeMode = false;

        bool manualMode = false;
    };
}