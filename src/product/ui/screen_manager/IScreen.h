#pragma once

#include "ScreenContext.h"

namespace incubator::ui
{
    class IScreen
    {
    public:
        virtual ~IScreen() = default;

        virtual void onEnter() = 0;

        virtual void onExit() = 0;

        virtual void render(
            const ScreenContext& context) = 0;
    };
}
