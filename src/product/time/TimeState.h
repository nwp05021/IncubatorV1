#pragma once

#include <stdint.h>

namespace incubator::time
{
    struct TimeState
    {
        bool rtcValid = false;

        bool ntpSynced = false;

        uint32_t currentEpoch = 0;

        uint32_t lastNtpSyncEpoch = 0;
    };
}