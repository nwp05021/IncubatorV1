#pragma once

#include <stdint.h>

#include "IScreen.h"
#include "ScreenId.h"

namespace incubator::ui
{
    class ScreenManager
    {
    public:
        bool registerScreen(
            ScreenId id,
            IScreen& screen);

        void request(
            ScreenId id);

        void tick(
            const ScreenContext& context);

        ScreenId activeScreen() const;

        ScreenId requestedScreen() const;

    private:
        static constexpr uint8_t MaxScreens =
            static_cast<uint8_t>(ScreenId::Count);

    private:
        void applyPendingTransition();

        uint8_t indexOf(
            ScreenId id) const;

    private:
        IScreen* m_screens[MaxScreens] = {};

        ScreenId m_active = ScreenId::Home;
        ScreenId m_requested = ScreenId::Home;

        bool m_hasActiveScreen = false;
    };
}
