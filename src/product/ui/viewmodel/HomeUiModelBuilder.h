#pragma once

#include "../../domain/RuntimeState.h"
#include "../model/HomeUiModel.h"

namespace incubator::ui
{
    class HomeUiModelBuilder
    {
    public:
        void build(
            const incubator::domain::RuntimeState& runtime,
            HomeUiModel& model);
    };
}
