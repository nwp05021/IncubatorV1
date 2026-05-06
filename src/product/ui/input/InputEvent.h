#pragma once

namespace incubator::ui
{
    enum class InputEventType
    {
        None,

        RotateLeft,
        RotateRight,

        Click,
        LongClick,
        LongHold
    };

    struct InputEvent
    {
        InputEventType type =
            InputEventType::None;
    };
}