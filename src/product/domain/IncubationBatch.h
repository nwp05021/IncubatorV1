#pragma once

#include <stdint.h>

namespace incubator::domain
{
    struct IncubationBatch
    {
        bool active = false;

        uint32_t startEpoch = 0;

        uint16_t totalDays = 21;

        uint16_t lockdownStartDay = 19;

        char batchId[32] = {0};
    };
}