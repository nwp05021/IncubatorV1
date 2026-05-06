#include "TemperatureCardWidget.h"

#include "../theme/UiTheme.h"

namespace incubator::ui
{
    TemperatureCardWidget::TemperatureCardWidget(
        incubator::devices::IDisplayDevice& display)
        :
        m_display(display)
    {
    }

    void TemperatureCardWidget::render(
        const HomeLayout& layout,
        const HomeUiModel& model)
    {
        const Rect& r =
            layout.tempCard;

        m_display.fillRect(
            r.x,
            r.y,
            r.w,
            r.h,
            UiTheme::Surface2);

        m_display.drawText(
            r.x + 10,
            r.y + 10,
            "TEMP",
            UiTheme::MutedText,
            UiTheme::Surface2,
            1);

        m_display.drawFloat(
            r.x + 10,
            r.y + 34,
            model.tempC,
            1,
            UiTheme::Text,
            UiTheme::Surface2,
            4);

        m_display.drawText(
            r.x + 10,
            r.y + 88,
            model.heaterOn ?
            "HEATER ON" :
            "HEATER OFF",
            model.heaterOn ?
            UiTheme::Warning :
            UiTheme::MutedText,
            UiTheme::Surface2,
            1);
    }
}
