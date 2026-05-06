#pragma once

#include <stdint.h>

namespace incubator::domain
{
    enum class SystemMode
    {
        Boot,
        Auto,
        Manual,
        SafeMode
    };

    struct RuntimeState
    {
        // ---------- System ----------

        SystemMode mode =
            SystemMode::Boot;

        bool safeMode = false;

        bool sensorHealthy = false;

        bool storageHealthy = false;

        // ---------- Temperature ----------

        float currentTempC = 0.0f;

        float targetTempC = 37.5f;

        // ---------- Humidity ----------

        float currentHumidityPct = 0.0f;

        float targetHumidityPct = 60.0f;

        // ---------- Output ----------

        bool heaterOn = false;

        bool humidifierOn = false;

        bool turnerOn = false;

        uint8_t fanPwm = 0;

        // ---------- Schedule ----------

        uint16_t currentDay = 0;

        uint16_t totalDays = 21;

        bool lockdown = false;

        // ---------- Alarm ----------

        bool highTempAlarm = false;

        bool lowTempAlarm = false;

        // ---------- Connectivity ----------

        bool wifiConnected = false;

        bool awsConnected = false;

        // ---------- Runtime ----------

        uint32_t uptimeMs = 0;
    };
}