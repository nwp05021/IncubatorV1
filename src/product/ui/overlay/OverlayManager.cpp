#include "OverlayManager.h"

namespace incubator::ui
{
    using namespace incubator::domain;

    OverlayType OverlayManager::resolve(
        const RuntimeState& runtime,
        const AlarmState& alarm)
    {
        if (runtime.safeMode)
        {
            return OverlayType::SafeMode;
        }

        if (alarm.highTemp ||
            alarm.lowTemp)
        {
            return OverlayType::Alarm;
        }

        return OverlayType::None;
    }
}
