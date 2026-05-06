#pragma once

#include "IDisplayDevice.h"

#include <LovyanGFX.hpp>

namespace incubator::devices
{
    class St7789DisplayDevice :
        public IDisplayDevice
    {
    public:
        bool begin() override;

        void beginFrame() override;

        void endFrame() override;

        void clear(
            uint16_t color) override;

        void fillRect(
            int x,
            int y,
            int w,
            int h,
            uint16_t color) override;

        void drawText(
            int x,
            int y,
            const char* text,
            uint16_t color,
            uint16_t bgColor,
            int textSize) override;

        void drawFloat(
            int x,
            int y,
            float value,
            int decimals,
            uint16_t color,
            uint16_t bgColor,
            int textSize) override;

        void drawProgressBar(
            int x,
            int y,
            int w,
            int h,
            int percent,
            uint16_t fgColor,
            uint16_t bgColor) override;

    private:
        class Panel :
            public lgfx::LGFX_Device
        {
        public:
            Panel();

        private:
            lgfx::Panel_ST7789 m_panel;
            lgfx::Bus_SPI m_bus;
        };

    private:
        Panel m_lcd;
    };
}
