#pragma once

#include <stdint.h>

namespace incubator::devices
{
    class IDisplayDevice
    {
    public:
        virtual ~IDisplayDevice() = default;

        virtual bool begin() = 0;

        virtual void beginFrame() = 0;

        virtual void endFrame() = 0;

        virtual void clear(
            uint16_t color) = 0;

        virtual void fillRect(
            int x,
            int y,
            int w,
            int h,
            uint16_t color) = 0;

        virtual void drawText(
            int x,
            int y,
            const char* text,
            uint16_t color,
            uint16_t bgColor,
            int textSize) = 0;

        virtual void drawFloat(
            int x,
            int y,
            float value,
            int decimals,
            uint16_t color,
            uint16_t bgColor,
            int textSize) = 0;

        virtual void drawProgressBar(
            int x,
            int y,
            int w,
            int h,
            int percent,
            uint16_t fgColor,
            uint16_t bgColor) = 0;
    };
}
