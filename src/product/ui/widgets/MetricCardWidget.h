#pragma once

#include "../../devices/display/IDisplayDevice.h"
#include "../common/Rect.h"

#include <stdint.h>

namespace incubator::ui
{
    struct MetricCardModel
    {
        const char* title = "";
        float value = 0.0f;
        int decimals = 1;
        const char* unit = "";
        const char* statusText = "";
        uint16_t accentColor = 0;
        uint16_t statusColor = 0;
        bool focused = false;
    };

    class MetricCardWidget
    {
    public:
        explicit MetricCardWidget(
            incubator::devices::IDisplayDevice& display);

    public:
        void render(
            const Rect& rect,
            const MetricCardModel& model);

    private:
        incubator::devices::IDisplayDevice& m_display;
    };
}
