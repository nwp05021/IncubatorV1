#pragma once

namespace incubator::ui
{
    struct HomeUiModel
    {
        float tempC = 0.0f;

        float humidityPct = 0.0f;

        bool heaterOn = false;

        bool humidifierOn = false;

        bool wifiConnected = false;

        bool awsConnected = false;

        bool safeMode = false;

        int currentDay = 0;
    };
}
