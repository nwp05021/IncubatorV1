#pragma once

namespace incubator::cloud
{
    struct TelemetryPayload
    {
        float currentTempC = 0.0f;

        float currentHumidityPct = 0.0f;

        float targetTempC = 0.0f;

        float targetHumidityPct = 0.0f;

        bool heaterOn = false;

        bool humidifierOn = false;

        bool safeMode = false;

        uint16_t currentDay = 0;

        bool wifiConnected = false;

        bool awsConnected = false;
    };
}