#include "PlanEditorUiModelBuilder.h"

namespace incubator::ui
{
    using namespace incubator::domain;

    void PlanEditorUiModelBuilder::build(
        const PlanRow& row,
        PlanEditorUiModel& model)
    {
        model.day =
            row.day;

        model.targetTempC =
            row.targetTempC;

        model.targetHumidityPct =
            row.targetHumidityPct;

        model.turningEnabled =
            row.turningEnabled;

        model.turningIntervalMs =
            row.turningIntervalMs;
    }
}