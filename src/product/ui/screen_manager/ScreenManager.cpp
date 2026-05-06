#include "ScreenManager.h"

namespace incubator::ui
{
    bool ScreenManager::registerScreen(
        ScreenId id,
        IScreen& screen)
    {
        const uint8_t index =
            indexOf(id);

        if (index >= MaxScreens)
        {
            return false;
        }

        m_screens[index] = &screen;

        return true;
    }

    void ScreenManager::request(
        ScreenId id)
    {
        const uint8_t index =
            indexOf(id);

        if (index >= MaxScreens)
        {
            return;
        }

        if (m_screens[index] == nullptr)
        {
            return;
        }

        m_requested = id;
    }

    void ScreenManager::tick(
        const ScreenContext& context)
    {
        applyPendingTransition();

        const uint8_t index =
            indexOf(m_active);

        if (index >= MaxScreens)
        {
            return;
        }

        IScreen* screen =
            m_screens[index];

        if (screen == nullptr)
        {
            return;
        }

        screen->render(context);
    }

    ScreenId ScreenManager::activeScreen() const
    {
        return m_active;
    }

    ScreenId ScreenManager::requestedScreen() const
    {
        return m_requested;
    }

    void ScreenManager::applyPendingTransition()
    {
        if (!m_hasActiveScreen)
        {
            const uint8_t index =
                indexOf(m_active);

            if (index < MaxScreens &&
                m_screens[index] != nullptr)
            {
                m_screens[index]->onEnter();
                m_hasActiveScreen = true;
            }

            return;
        }

        if (m_requested == m_active)
        {
            return;
        }

        const uint8_t activeIndex =
            indexOf(m_active);

        if (activeIndex < MaxScreens &&
            m_screens[activeIndex] != nullptr)
        {
            m_screens[activeIndex]->onExit();
        }

        m_active = m_requested;

        const uint8_t nextIndex =
            indexOf(m_active);

        if (nextIndex < MaxScreens &&
            m_screens[nextIndex] != nullptr)
        {
            m_screens[nextIndex]->onEnter();
        }
    }

    uint8_t ScreenManager::indexOf(
        ScreenId id) const
    {
        return static_cast<uint8_t>(id);
    }
}
