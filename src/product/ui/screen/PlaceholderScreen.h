#pragma once

#include "../screen_manager/IScreen.h"
#include "../screen_manager/ScreenId.h"
#include "../../devices/display/IDisplayDevice.h"

namespace incubator::ui
{
    class PlaceholderScreen :
        public IScreen
    {
    public:
        PlaceholderScreen(
            incubator::devices::IDisplayDevice& display,
            ScreenId id,
            const char* title,
            const char* subtitle);

    public:
        void onEnter() override;

        void onExit() override;

        void render(
            const ScreenContext& context) override;

    private:
        const char* screenName() const;

    private:
        incubator::devices::IDisplayDevice& m_display;
        ScreenId m_id;
        const char* m_title = "";
        const char* m_subtitle = "";
        bool m_dirty = true;
    };
}
