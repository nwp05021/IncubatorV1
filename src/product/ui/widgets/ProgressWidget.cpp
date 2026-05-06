#include "ProgressWidget.h"

#include "../theme/UiTheme.h"

#include <stdio.h>

namespace incubator::ui
{
    ProgressWidget::ProgressWidget(
        incubator::devices::IDisplayDevice& display)
        :
        m_display(display)
    {
    }

    void ProgressWidget::render(
        const HomeLayout& layout,
        const HomeUiModel& model)
    {
        const Rect& r =
            layout.progress;

        int percent = 0;

        if (model.totalDays > 0)
        {
            percent =
                (model.currentDay * 100) /
                model.totalDays;
        }

        if (percent < 0)
        {
            percent = 0;
        }

        if (percent > 100)
        {
            percent = 100;
        }

        char dayText[24];

        snprintf(
            dayText,
            sizeof(dayText),
            "DAY %d/%d",
            model.currentDay,
            model.totalDays);

        m_display.fillRect(
            r.x,
            r.y,
            r.w,
            r.h,
            UiTheme::Background);

        m_display.drawText(
            r.x + 8,
            r.y + 6,
            "INCUBATION PROGRESS",
            UiTheme::MutedText,
            UiTheme::Background,
            1);

        m_display.drawText(
            r.x + r.w - 76,
            r.y + 6,
            dayText,
            UiTheme::Text,
            UiTheme::Background,
            1);

        m_display.drawProgressBar(
            r.x + 8,
            r.y + 28,
            r.w - 16,
            14,
            percent,
            UiTheme::Accent,
            UiTheme::Surface);
    }
}
