#pragma once

#include <stdint.h>

namespace incubator::logging
{
    class SerialLogger
    {
    public:
        static constexpr uint16_t BufferSize = 2048;

    public:
        void begin(
            uint32_t baudRate);

        bool write(
            const char* text);

        bool writeLine(
            const char* text);

        void tick(
            uint32_t nowMs);

        uint16_t available() const;

        uint16_t dropped() const;

    private:
        bool pushChar(
            char c);

        bool popChar(
            char& c);

    private:
        char m_buffer[BufferSize];

        uint16_t m_head = 0;

        uint16_t m_tail = 0;

        uint16_t m_count = 0;

        uint16_t m_dropped = 0;

        uint32_t m_lastFlushMs = 0;

        static constexpr uint16_t MaxCharsPerTick =
            96;

        static constexpr uint32_t FlushIntervalMs =
            10;
    };
}
