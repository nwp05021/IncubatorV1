#pragma once

#include "../model/HomeUiModel.h"
#include "HomeDirtyFlags.h"

namespace incubator::ui
{
    class HomeUiDiff
    {
    public:
        HomeDirtyFlags diff(
            const HomeUiModel& previous,
            const HomeUiModel& current,
            bool firstRender) const;

    private:
        bool changedFloat(
            float a,
            float b,
            float threshold) const;
    };
}
