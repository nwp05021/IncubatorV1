#pragma once

#include "../../domain/AlarmState.h"

namespace incubator::ui
{
    class AlarmOverlayRenderer
    {
    public:
        void render(
            const incubator::domain::AlarmState& alarm);
    };
}