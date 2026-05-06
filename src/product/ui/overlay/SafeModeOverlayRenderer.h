#pragma once

#include "../../domain/RecoveryState.h"

namespace incubator::ui
{
    class SafeModeOverlayRenderer
    {
    public:
        void render(
            incubator::domain::SafeModeReason reason);
    };
}