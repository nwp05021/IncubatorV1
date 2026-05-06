#pragma once

#include "../../devices/display/IDisplayDevice.h"

#include "../model/HomeUiModel.h"
#include "../dirty/HomeDirtyFlags.h"

namespace incubator::ui
{
    class GraphicHomeRenderer
    {
    public:
        explicit GraphicHomeRenderer(
            incubator::devices::IDisplayDevice& display);

    public:
        void render(
            const HomeUiModel& model,
            const HomeDirtyFlags& dirty);

    private:
        void renderStatusBar(
            const HomeUiModel& model);

        void renderTemperature(
            const HomeUiModel& model);

        void renderHumidity(
            const HomeUiModel& model);

        void renderOutputs(
            const HomeUiModel& model);

        void renderProgress(
            const HomeUiModel& model);

        void renderOverlay(
            const HomeUiModel& model);

    private:
        incubator::devices::IDisplayDevice& m_display;
    };
}
