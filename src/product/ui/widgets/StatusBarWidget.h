#pragma once

#include "../../devices/display/IDisplayDevice.h"

#include "../layout/HomeLayout.h"
#include "../model/HomeUiModel.h"

namespace incubator::ui
{
    class StatusBarWidget
    {
    public:
        explicit StatusBarWidget(
            incubator::devices::IDisplayDevice& display);

    public:
        void render(
            const HomeLayout& layout,
            const HomeUiModel& model);

    private:
        incubator::devices::IDisplayDevice& m_display;
    };
}
