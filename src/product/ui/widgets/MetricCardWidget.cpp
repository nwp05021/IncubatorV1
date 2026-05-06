#include "MetricCardWidget.h"

#include "../theme/UiTheme.h"

namespace incubator::ui
{
    MetricCardWidget::MetricCardWidget(
        incubator::devices::IDisplayDevice& display)
        :
        m_display(display)
    {
    }

    void MetricCardWidget::render(
        const Rect& rect,
        const MetricCardModel& model)
    {
        const uint16_t surfaceColor =
            model.focused ? UiTheme::Surface : UiTheme::Surface2;

        m_display.fillRect(
            rect.x,
            rect.y,
            rect.w,
            rect.h,
            surfaceColor);

        if (model.focused)
        {
            m_display.fillRect(rect.x, rect.y, rect.w, 3, UiTheme::Accent);
            m_display.fillRect(rect.x, rect.y + rect.h - 3, rect.w, 3, UiTheme::Accent);
            m_display.fillRect(rect.x, rect.y, 3, rect.h, UiTheme::Accent);
            m_display.fillRect(rect.x + rect.w - 3, rect.y, 3, rect.h, UiTheme::Accent);
        }

        m_display.drawText(
            rect.x + 10,
            rect.y + 10,
            model.title,
            UiTheme::MutedText,
            surfaceColor,
            1);

        m_display.drawFloat(
            rect.x + 10,
            rect.y + 34,
            model.value,
            model.decimals,
            UiTheme::Text,
            surfaceColor,
            4);

        m_display.drawText(
            rect.x + rect.w - 44,
            rect.y + 50,
            model.unit,
            model.accentColor,
            surfaceColor,
            2);

        m_display.drawText(
            rect.x + 10,
            rect.y + rect.h - 20,
            model.statusText,
            model.statusColor,
            surfaceColor,
            1);
    }
}
