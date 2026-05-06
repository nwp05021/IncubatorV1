#pragma once

#include <stdint.h>

namespace incubator::ui
{
    struct UiTheme
    {
        static constexpr uint16_t Background = 0x0000;
        static constexpr uint16_t Surface = 0x2104;
        static constexpr uint16_t Surface2 = 0x3186;
        static constexpr uint16_t Text = 0xFFFF;
        static constexpr uint16_t MutedText = 0x8410;
        static constexpr uint16_t Accent = 0x07FF;
        static constexpr uint16_t Warning = 0xFFE0;
        static constexpr uint16_t Danger = 0xF800;
        static constexpr uint16_t Good = 0x07E0;
    };
}
