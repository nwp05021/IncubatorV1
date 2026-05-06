#pragma once

#include "../model/PlanEditorUiModel.h"

namespace incubator::ui
{
    class PlanEditorRenderer
    {
    public:
        void render(
            const PlanEditorUiModel& model);
    };
}