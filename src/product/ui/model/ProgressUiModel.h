#pragma once

#include <stdint.h>

namespace incubator::ui
{
    struct ProgressUiModel
    {
        uint16_t currentDay = 0;

        uint16_t totalDays = 21;

        bool lockdown = false;

        float currentTempC = 0.0f;

        float currentHumidityPct = 0.0f;

        uint8_t progressPct = 0;
    };
}