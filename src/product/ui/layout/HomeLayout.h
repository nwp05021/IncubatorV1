#pragma once

#include "../common/Rect.h"

namespace incubator::ui
{
    struct HomeLayout
    {
        Rect statusBar =
        {
            0, 0, 320, 28
        };

        Rect tempCard =
        {
            0, 32, 156, 108
        };

        Rect humidityCard =
        {
            164, 32, 156, 108
        };

        Rect outputBar =
        {
            0, 146, 320, 38
        };

        Rect progress =
        {
            0, 190, 320, 50
        };

        Rect overlay =
        {
            36, 70, 248, 100
        };
    };
}
