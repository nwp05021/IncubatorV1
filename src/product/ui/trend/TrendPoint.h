#pragma once

#include <stdint.h>

namespace incubator::ui
{
    struct TrendPoint
    {
        float value = 0.0f;

        uint32_t timestampMs = 0;
    };
}