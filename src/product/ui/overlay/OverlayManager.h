#pragma once

#include "../../domain/RuntimeState.h"
#include "../../domain/AlarmState.h"

namespace incubator::ui
{
    enum class OverlayType
    {
        None,
        Alarm,
        SafeMode
    };

    class OverlayManager
    {
    public:
        OverlayType resolve(
            const incubator::domain::RuntimeState& runtime,
            const incubator::domain::AlarmState& alarm);
    };
}
