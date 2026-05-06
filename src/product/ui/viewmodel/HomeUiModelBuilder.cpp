#include "HomeUiModelBuilder.h"

namespace incubator::ui
{
    using namespace incubator::domain;

    void HomeUiModelBuilder::build(
        const RuntimeState& runtime,
        HomeUiModel& model)
    {
        model.tempC =
            runtime.currentTempC;

        model.humidityPct =
            runtime.currentHumidityPct;

        model.heaterOn =
            runtime.heaterOn;

        model.humidifierOn =
            runtime.humidifierOn;

        model.wifiConnected =
            runtime.wifiConnected;

        model.awsConnected =
            runtime.awsConnected;

        model.safeMode =
            runtime.safeMode;

        model.currentDay =
            runtime.currentDay;

        model.totalDays =
            runtime.totalDays;

        model.fanPwm =
            runtime.fanPwm;

        model.fanOn =
            runtime.fanPwm > 0;

        model.highTempAlarm =
            runtime.highTempAlarm;

        model.lowTempAlarm =
            runtime.lowTempAlarm;

        model.focusedItem =
            runtime.focusedItem;
    }
}
