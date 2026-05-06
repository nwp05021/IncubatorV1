#include "HomeUiDiff.h"

#include <math.h>

namespace incubator::ui
{
    HomeDirtyFlags HomeUiDiff::diff(
        const HomeUiModel& previous,
        const HomeUiModel& current,
        bool firstRender) const
    {
        HomeDirtyFlags flags;

        if (firstRender)
        {
            flags.markAll();

            return flags;
        }

        flags.clear();

        flags.focusChanged =
            previous.focusedItem != current.focusedItem;

        flags.statusBar =
            previous.wifiConnected != current.wifiConnected ||
            previous.awsConnected != current.awsConnected ||
            previous.safeMode != current.safeMode;

        flags.temperature =
            changedFloat(
                previous.tempC,
                current.tempC,
                0.05f) ||
            previous.heaterOn != current.heaterOn ||
            previous.highTempAlarm != current.highTempAlarm ||
            previous.lowTempAlarm != current.lowTempAlarm ||
            previous.focusedItem == incubator::domain::UiFocusId::Temperature ||
            current.focusedItem == incubator::domain::UiFocusId::Temperature;

        flags.humidity =
            changedFloat(
                previous.humidityPct,
                current.humidityPct,
                0.2f) ||
            previous.humidifierOn != current.humidifierOn ||
            previous.focusedItem == incubator::domain::UiFocusId::Humidity ||
            current.focusedItem == incubator::domain::UiFocusId::Humidity;

        flags.outputs =
            previous.heaterOn != current.heaterOn ||
            previous.humidifierOn != current.humidifierOn ||
            previous.fanOn != current.fanOn ||
            previous.fanPwm != current.fanPwm ||
            previous.focusedItem == incubator::domain::UiFocusId::Fan ||
            current.focusedItem == incubator::domain::UiFocusId::Fan ||
            previous.focusedItem == incubator::domain::UiFocusId::StartButton ||
            current.focusedItem == incubator::domain::UiFocusId::StartButton ||
            previous.focusedItem == incubator::domain::UiFocusId::StopButton ||
            current.focusedItem == incubator::domain::UiFocusId::StopButton;

        flags.progress =
            previous.currentDay != current.currentDay ||
            previous.totalDays != current.totalDays;

        flags.overlay =
            previous.safeMode != current.safeMode ||
            previous.highTempAlarm != current.highTempAlarm ||
            previous.lowTempAlarm != current.lowTempAlarm;

        return flags;
    }

    bool HomeUiDiff::changedFloat(
        float a,
        float b,
        float threshold) const
    {
        return fabsf(a - b) >= threshold;
    }
}
