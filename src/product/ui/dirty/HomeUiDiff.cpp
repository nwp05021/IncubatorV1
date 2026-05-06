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
            previous.lowTempAlarm != current.lowTempAlarm;

        flags.humidity =
            changedFloat(
                previous.humidityPct,
                current.humidityPct,
                0.2f) ||
            previous.humidifierOn != current.humidifierOn;

        flags.outputs =
            previous.heaterOn != current.heaterOn ||
            previous.humidifierOn != current.humidifierOn ||
            previous.fanOn != current.fanOn ||
            previous.fanPwm != current.fanPwm;

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
