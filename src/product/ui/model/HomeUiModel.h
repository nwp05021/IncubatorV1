#pragma once

#include <stdint.h>

#include "../../domain/UiFocusId.h"

namespace incubator::ui
{
    struct HomeUiModel
    {
        float tempC = 0.0f;

        float humidityPct = 0.0f;

        bool heaterOn = false;

        bool humidifierOn = false;

        bool fanOn = false;

        uint8_t fanPwm = 0;

        bool wifiConnected = false;

        bool awsConnected = false;

        bool safeMode = false;

        bool highTempAlarm = false;

        bool lowTempAlarm = false;

        uint16_t currentDay = 0;

        uint16_t totalDays = 21;

        incubator::domain::UiFocusId focusedItem =
            incubator::domain::UiFocusId::Temperature;
    };
}
