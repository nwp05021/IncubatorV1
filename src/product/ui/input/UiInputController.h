#pragma once

#include "InputEvent.h"

namespace incubator::ui
{
    class UiInputController
    {
    public:
        bool poll(InputEvent& event);
    };
}