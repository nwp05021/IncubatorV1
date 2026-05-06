#pragma once

#include "../model/ProgressUiModel.h"
#include "../trend/TrendBuffer.h"

namespace incubator::ui
{
    class ProgressRenderer
    {
    public:
        void render(
            const ProgressUiModel& model,
            const TrendBuffer& tempTrend,
            const TrendBuffer& humidityTrend);

    private:
        void renderProgressBar(
            const ProgressUiModel& model);

        void renderTrendGraph(
            const TrendBuffer& trend);
    };
}