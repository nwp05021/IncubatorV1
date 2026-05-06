#pragma once

namespace incubator::power
{
    struct PowerState
    {
        bool brownoutDetected = false;

        bool watchdogTriggered = false;

        bool powerHealthy = true;

        float inputVoltage = 0.0f;
    };
}