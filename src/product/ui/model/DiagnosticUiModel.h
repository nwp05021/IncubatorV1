#pragma once

namespace incubator::ui
{
    struct DiagnosticUiModel
    {
        bool sensorHealthy = false;

        bool storageHealthy = false;

        bool wifiConnected = false;

        bool mqttConnected = false;

        bool safeMode = false;
    };
}