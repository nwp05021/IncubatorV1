#pragma once

#include "../../devices/display/IDisplayDevice.h"
#include "../common/Rect.h"
#include "../model/HomeUiModel.h"

namespace incubator::ui
{
    class OutputBarWidget
    {
    public:
        explicit OutputBarWidget(
            incubator::devices::IDisplayDevice& display);

    public:
        void render(
            const Rect& rect,
            const HomeUiModel& model);

    private:
        incubator::devices::IDisplayDevice& m_display;
    };
}
