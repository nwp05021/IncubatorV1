#include "OutputBarWidget.h"

#include "../theme/UiTheme.h"

namespace incubator::ui
{
    using incubator::domain::UiFocusId;

    OutputBarWidget::OutputBarWidget(
        incubator::devices::IDisplayDevice& display)
        :
        m_display(display)
    {
    }

    void OutputBarWidget::render(
        const Rect& rect,
        const HomeUiModel& model)
    {
        const bool fanFocused =
            model.focusedItem == UiFocusId::Fan;

        const bool startFocused =
            model.focusedItem == UiFocusId::StartButton;

        const bool stopFocused =
            model.focusedItem == UiFocusId::StopButton;

        m_display.fillRect(
            rect.x,
            rect.y,
            rect.w,
            rect.h,
            UiTheme::Surface);

        if (fanFocused || startFocused || stopFocused)
        {
            m_display.fillRect(rect.x, rect.y, rect.w, 3, UiTheme::Accent);
            m_display.fillRect(rect.x, rect.y + rect.h - 3, rect.w, 3, UiTheme::Accent);
        }

        m_display.drawText(
            rect.x + 8,
            rect.y + 12,
            "OUTPUT",
            UiTheme::MutedText,
            UiTheme::Surface,
            1);

        m_display.drawText(
            rect.x + 80,
            rect.y + 12,
            model.heaterOn ? "HEAT" : "----",
            model.heaterOn ? UiTheme::Warning : UiTheme::MutedText,
            UiTheme::Surface,
            1);

        m_display.drawText(
            rect.x + 140,
            rect.y + 12,
            model.humidifierOn ? "HUMI" : "----",
            model.humidifierOn ? UiTheme::Accent : UiTheme::MutedText,
            UiTheme::Surface,
            1);

        m_display.drawText(
            rect.x + 200,
            rect.y + 12,
            model.fanOn ? "FAN" : "---",
            fanFocused ? UiTheme::Accent : (model.fanOn ? UiTheme::Good : UiTheme::MutedText),
            UiTheme::Surface,
            1);

        m_display.drawText(
            rect.x + 248,
            rect.y + 12,
            startFocused ? "START" : (stopFocused ? "STOP" : "READY"),
            (startFocused || stopFocused) ? UiTheme::Accent : UiTheme::MutedText,
            UiTheme::Surface,
            1);
    }
}
