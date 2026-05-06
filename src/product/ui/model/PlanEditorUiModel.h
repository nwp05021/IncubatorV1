#pragma once

#include <stdint.h>

namespace incubator::ui
{
    struct PlanEditorUiModel
    {
        uint16_t day = 1;

        float targetTempC = 37.5f;

        float targetHumidityPct = 60.0f;

        bool turningEnabled = true;

        uint32_t turningIntervalMs = 7200000;

        bool modified = false;
    };
}