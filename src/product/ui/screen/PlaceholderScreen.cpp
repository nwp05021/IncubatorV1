#include "PlaceholderScreen.h"

#include "../theme/UiTheme.h"

namespace incubator::ui
{
    PlaceholderScreen::PlaceholderScreen(
        incubator::devices::IDisplayDevice& display,
        ScreenId id,
        const char* title,
        const char* subtitle)
        :
        m_display(display),
        m_id(id),
        m_title(title),
        m_subtitle(subtitle)
    {
    }

    void PlaceholderScreen::onEnter()
    {
        m_dirty = true;
    }

    void PlaceholderScreen::onExit()
    {
    }

    void PlaceholderScreen::render(
        const ScreenContext& context)
    {
        (void)context;

        if (!m_dirty)
        {
            return;
        }

        const bool spriteFrameStarted =
            m_display.beginSpriteFrame(UiTheme::Background);

        if (!spriteFrameStarted)
        {
            m_display.beginFrame();
            m_display.clear(UiTheme::Background);
        }

        m_display.fillRect(
            0,
            0,
            320,
            28,
            UiTheme::Surface2);

        m_display.drawText(
            10,
            7,
            screenName(),
            UiTheme::Text,
            UiTheme::Surface2,
            1);

        m_display.fillRect(
            16,
            48,
            288,
            128,
            UiTheme::Surface);

        m_display.drawText(
            32,
            74,
            m_title,
            UiTheme::Text,
            UiTheme::Surface,
            2);

        m_display.drawText(
            32,
            112,
            m_subtitle,
            UiTheme::MutedText,
            UiTheme::Surface,
            1);

        m_display.drawText(
            32,
            146,
            "ScreenManager Ready",
            UiTheme::Accent,
            UiTheme::Surface,
            1);

        if (spriteFrameStarted)
        {
            m_display.endSpriteFrame();
        }
        else
        {
            m_display.endFrame();
        }

        m_dirty = false;
    }

    const char* PlaceholderScreen::screenName() const
    {
        switch (m_id)
        {
            case ScreenId::Progress:
                return "PROGRESS";

            case ScreenId::Settings:
                return "SETTINGS";

            case ScreenId::Diagnostics:
                return "DIAGNOSTICS";

            case ScreenId::Alarm:
                return "ALARM";

            case ScreenId::Home:
                return "HOME";

            default:
                return "SCREEN";
        }
    }
}
