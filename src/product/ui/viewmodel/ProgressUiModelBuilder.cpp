#include "ProgressUiModelBuilder.h"

namespace incubator::ui
{
    using namespace incubator::domain;

    void ProgressUiModelBuilder::build(
        const RuntimeState& runtime,
        ProgressUiModel& model)
    {
        model.currentDay =
            runtime.currentDay;

        model.totalDays =
            runtime.totalDays;

        model.lockdown =
            runtime.lockdown;

        model.currentTempC =
            runtime.currentTempC;

        model.currentHumidityPct =
            runtime.currentHumidityPct;

        if (runtime.totalDays > 0)
        {
            model.progressPct =
                static_cast<uint8_t>(
                    (runtime.currentDay * 100) /
                    runtime.totalDays);
        }
    }
}