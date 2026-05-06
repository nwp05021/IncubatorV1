#pragma once

#include "../model/HomeUiModel.h"

namespace incubator::ui
{
    class HomeRenderer
    {
    public:
        void render(
            const HomeUiModel& model);

    private:
        void renderStatusBar(
            const HomeUiModel& model);

        void renderMainValues(
            const HomeUiModel& model);

        void renderOutputs(
            const HomeUiModel& model);
    };
}
