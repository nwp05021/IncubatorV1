#pragma once

#include "../../domain/RuntimeState.h"
#include "../model/ProgressUiModel.h"

namespace incubator::ui
{
    class ProgressUiModelBuilder
    {
    public:
        void build(
            const incubator::domain::RuntimeState& runtime,
            ProgressUiModel& model);
    };
}