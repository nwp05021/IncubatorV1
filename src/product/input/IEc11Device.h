#pragma once

#include <stdint.h>
#include "InputEvent.h"

namespace incubator::input
{
    class IEc11Device
    {
    public:
        virtual ~IEc11Device() = default;

        virtual void begin() = 0;

        virtual bool poll(
            uint32_t nowMs,
            InputEvent& event) = 0;
    };
}
