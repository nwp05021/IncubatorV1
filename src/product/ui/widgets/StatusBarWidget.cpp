#include "StatusBarWidget.h"

#include "../theme/UiTheme.h"

namespace incubator::ui
{
    StatusBarWidget::StatusBarWidget(
        incubator::devices::IDisplayDevice& display)
        :
        m_display(display)
    {
    }

    void StatusBarWidget::render(
        const HomeLayout& layout,
        const HomeUiModel& model)
    {
        const Rect& r =
            layout.statusBar;

        m_display.fillRect(
            r.x,
            r.y,
            r.w,
            r.h,
            UiTheme::Surface);

        m_display.drawText(
            r.x + 8,
            r.y + 7,
            "INCUBATOR",
            UiTheme::Text,
            UiTheme::Surface,
            2);

        m_display.drawText(
            r.x + 190,
            r.y + 7,
            model.wifiConnected ? "WIFI" : "NO WIFI",
            model.wifiConnected ? UiTheme::Good : UiTheme::Warning,
            UiTheme::Surface,
            1);

        m_display.drawText(
            r.x + 260,
            r.y + 7,
            model.awsConnected ? "AWS" : "OFF",
            model.awsConnected ? UiTheme::Good : UiTheme::MutedText,
            UiTheme::Surface,
            1);
    }
}
