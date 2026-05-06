#pragma once

#include <stdint.h>

namespace incubator::performance
{
    struct PerformanceState
    {
        uint32_t loopCount = 0;
        uint32_t lastLoopTimeUs = 0;
        uint32_t maxLoopTimeUs = 0;
        uint32_t freeHeap = 0;
        uint32_t minFreeHeap = 0;
        bool loopOverrun = false;
    };
}
