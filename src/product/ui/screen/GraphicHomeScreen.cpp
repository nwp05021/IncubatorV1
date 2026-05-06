#include "GraphicHomeScreen.h"

namespace incubator::ui
{
    GraphicHomeScreen::GraphicHomeScreen(
        incubator::devices::IDisplayDevice& display)
        :
        m_renderer(display)
    {
    }

    void GraphicHomeScreen::render(
        const HomeUiModel& current)
    {
        HomeDirtyFlags dirty =
            m_diff.diff(
                m_previous,
                current,
                m_firstRender);

        m_renderer.render(
            current,
            dirty);

        if (dirty.any())
        {
            m_previous =
                current;

            m_firstRender =
                false;
        }
    }

    void GraphicHomeScreen::invalidate()
    {
        m_firstRender = true;
    }

    void GraphicHomeScreen::onEnter()
    {
        invalidate();
    }

    void GraphicHomeScreen::onExit()
    {
    }

    void GraphicHomeScreen::render(
        const ScreenContext& context)
    {
        if (context.home == nullptr)
        {
            return;
        }

        render(*context.home);
    }

}
