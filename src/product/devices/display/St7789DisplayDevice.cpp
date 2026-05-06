#include "St7789DisplayDevice.h"

#include <stdio.h>

namespace incubator::devices
{
    St7789DisplayDevice::Panel::Panel()
    {
        {
            auto cfg =
                m_bus.config();

            cfg.spi_host = SPI2_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = 40000000;
            cfg.freq_read = 16000000;
            cfg.spi_3wire = false;
            cfg.use_lock = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;

            // TODO: 실제 배선에 맞게 조정
            cfg.pin_sclk = 12;
            cfg.pin_mosi = 11;
            cfg.pin_miso = -1;
            cfg.pin_dc = 13;

            m_bus.config(cfg);
            m_panel.setBus(&m_bus);
        }

        {
            auto cfg =
                m_panel.config();

            cfg.pin_cs = 10;
            cfg.pin_rst = 14;
            cfg.pin_busy = -1;

            cfg.panel_width = 240;
            cfg.panel_height = 320;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            cfg.offset_rotation = 0;

            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits = 1;

            cfg.readable = false;
            cfg.invert = true;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = false;

            m_panel.config(cfg);
        }

        setPanel(&m_panel);
    }

    bool St7789DisplayDevice::begin()
    {
        m_lcd.init();
        m_lcd.setRotation(1);
        m_lcd.setBrightness(180);
        m_lcd.fillScreen(TFT_BLACK);

        m_sprite.setColorDepth(16);
        void* spriteBuffer =
            m_sprite.createSprite(320, 240);

        m_spriteReady =
            spriteBuffer != nullptr;

        return true;
    }

    void St7789DisplayDevice::beginFrame()
    {
        if (m_spriteFrameActive)
        {
            return;
        }

        m_lcd.startWrite();
    }

    void St7789DisplayDevice::endFrame()
    {
        if (m_spriteFrameActive)
        {
            return;
        }

        m_lcd.endWrite();
    }

    void St7789DisplayDevice::clear(
        uint16_t color)
    {
        if (m_spriteFrameActive)
        {
            m_sprite.fillScreen(color);
            return;
        }

        m_lcd.fillScreen(color);
    }

    void St7789DisplayDevice::fillRect(
        int x,
        int y,
        int w,
        int h,
        uint16_t color)
    {
        if (m_spriteFrameActive)
        {
            m_sprite.fillRect(
                x,
                y,
                w,
                h,
                color);
            return;
        }

        m_lcd.fillRect(
            x,
            y,
            w,
            h,
            color);
    }

    void St7789DisplayDevice::drawText(
        int x,
        int y,
        const char* text,
        uint16_t color,
        uint16_t bgColor,
        int textSize)
    {
        if (m_spriteFrameActive)
        {
            m_sprite.setTextSize(textSize);
            m_sprite.setTextColor(color, bgColor);
            m_sprite.setCursor(x, y);
            m_sprite.print(text);
            return;
        }

        m_lcd.setTextSize(textSize);
        m_lcd.setTextColor(color, bgColor);
        m_lcd.setCursor(x, y);
        m_lcd.print(text);
    }

    void St7789DisplayDevice::drawFloat(
        int x,
        int y,
        float value,
        int decimals,
        uint16_t color,
        uint16_t bgColor,
        int textSize)
    {
        char buffer[24];

        snprintf(
            buffer,
            sizeof(buffer),
            "%.*f",
            decimals,
            value);

        drawText(
            x,
            y,
            buffer,
            color,
            bgColor,
            textSize);
    }

    void St7789DisplayDevice::drawProgressBar(
        int x,
        int y,
        int w,
        int h,
        int percent,
        uint16_t fgColor,
        uint16_t bgColor)
    {
        if (percent < 0)
        {
            percent = 0;
        }

        if (percent > 100)
        {
            percent = 100;
        }

        fillRect(
            x,
            y,
            w,
            h,
            bgColor);

        const int filled =
            (w * percent) / 100;

        fillRect(
            x,
            y,
            filled,
            h,
            fgColor);
    }
    bool St7789DisplayDevice::beginSpriteFrame(
        uint16_t clearColor)
    {
        if (!m_spriteReady)
        {
            return false;
        }

        m_spriteFrameActive = true;
        m_sprite.fillScreen(clearColor);

        return true;
    }

    void St7789DisplayDevice::endSpriteFrame()
    {
        if (!m_spriteFrameActive)
        {
            return;
        }

        m_lcd.startWrite();
        m_sprite.pushSprite(0, 0);
        m_lcd.endWrite();

        m_spriteFrameActive = false;
    }

    bool St7789DisplayDevice::isSpriteFrameActive() const
    {
        return m_spriteFrameActive;
    }

}
