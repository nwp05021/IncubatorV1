#pragma once

#include "../event/EventRecord.h"

namespace incubator::ui
{
    class ToastManager
    {
    public:
        void push(
            const incubator::event::EventRecord& event);

        void tick(uint32_t nowMs);
    };
}