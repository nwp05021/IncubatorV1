#pragma once

#include "../../domain/PlanRow.h"
#include "../model/PlanEditorUiModel.h"

namespace incubator::ui
{
    class PlanEditorUiModelBuilder
    {
    public:
        void build(
            const incubator::domain::PlanRow& row,
            PlanEditorUiModel& model);
    };
}