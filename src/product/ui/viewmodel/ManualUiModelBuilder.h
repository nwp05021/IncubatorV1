#pragma once

#include "../../domain/RuntimeState.h"
#include "../model/ManualUiModel.h"

namespace incubator::ui
{
    class ManualUiModelBuilder
    {
    public:
        void build(
            const incubator::domain::RuntimeState& runtime,
            ManualUiModel& model);
    };
}