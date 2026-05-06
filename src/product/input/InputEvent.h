#pragma once

#include <stdint.h>

namespace incubator::input
{
    enum class InputEventType
    {
        None,
        RotateLeft,
        RotateRight,
        ButtonClick,
        ButtonDown,
        ButtonUp
    };

    enum class InputSource
    {
        Unknown,
        EC11
    };

    struct InputEvent
    {
        InputEventType type = InputEventType::None;
        InputSource source = InputSource::Unknown;
        int8_t delta = 0;
        uint32_t timestampMs = 0;
    };
}
