#pragma once

#include "../model/ManualUiModel.h"

namespace incubator::ui
{
    class ManualRenderer
    {
    public:
        void render(
            const ManualUiModel& model);
    };
}