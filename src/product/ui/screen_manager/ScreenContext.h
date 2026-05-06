#pragma once

#include <stdint.h>

#include "../model/HomeUiModel.h"

namespace incubator::ui
{
    struct ScreenContext
    {
        const HomeUiModel* home = nullptr;
        uint32_t nowMs = 0;
    };
}
