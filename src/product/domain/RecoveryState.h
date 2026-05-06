#pragma once

#include <stdint.h>

namespace incubator::domain
{
    enum class SafeModeReason
    {
        None,
        SensorFail,
        PlanCorrupt,
        StorageFail,
        WdtReset,
        CriticalAlarm
    };

    struct RecoveryState
    {
        uint32_t bootCount = 0;

        uint32_t wdtResetCount = 0;

        bool lastBootRecovered = false;

        SafeModeReason safeModeReason =
            SafeModeReason::None;
    };
}