#include "SerialLogger.h"

#include <Arduino.h>

namespace incubator::logging
{
    void SerialLogger::begin(
        uint32_t baudRate)
    {
        Serial.begin(
            baudRate);
    }

    bool SerialLogger::write(
        const char* text)
    {
        if (text == nullptr)
        {
            return false;
        }

        bool ok = true;

        while (*text != '\0')
        {
            if (!pushChar(*text))
            {
                ok = false;
            }

            ++text;
        }

        return ok;
    }

    bool SerialLogger::writeLine(
        const char* text)
    {
        bool ok =
            write(text);

        if (!pushChar('\n'))
        {
            ok = false;
        }

        return ok;
    }

    void SerialLogger::tick(
        uint32_t nowMs)
    {
        if ((nowMs - m_lastFlushMs) <
            FlushIntervalMs)
        {
            return;
        }

        m_lastFlushMs = nowMs;

        uint16_t sent = 0;

        char c = 0;

        while (sent < MaxCharsPerTick &&
               popChar(c))
        {
            Serial.write(c);

            ++sent;
        }
    }

    uint16_t SerialLogger::available() const
    {
        return m_count;
    }

    uint16_t SerialLogger::dropped() const
    {
        return m_dropped;
    }

    bool SerialLogger::pushChar(
        char c)
    {
        if (m_count >= BufferSize)
        {
            ++m_dropped;

            return false;
        }

        m_buffer[m_tail] = c;

        m_tail =
            (m_tail + 1) % BufferSize;

        ++m_count;

        return true;
    }

    bool SerialLogger::popChar(
        char& c)
    {
        if (m_count == 0)
        {
            return false;
        }

        c = m_buffer[m_head];

        m_head =
            (m_head + 1) % BufferSize;

        --m_count;

        return true;
    }
}
