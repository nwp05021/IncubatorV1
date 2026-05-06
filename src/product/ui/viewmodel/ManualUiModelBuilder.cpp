#include "ManualUiModelBuilder.h"

namespace incubator::ui
{
    using namespace incubator::domain;

    void ManualUiModelBuilder::build(
        const RuntimeState& runtime,
        ManualUiModel& model)
    {
        model.heaterOn =
            runtime.heaterOn;

        model.humidifierOn =
            runtime.humidifierOn;

        model.turnerOn =
            runtime.turnerOn;

        model.fanPwm =
            runtime.fanPwm;

        model.safeMode =
            runtime.safeMode;

        model.manualMode =
            (runtime.mode ==
             SystemMode::Manual);
    }
}