#include "Ec11InputDevice.h"

#include <Arduino.h>

namespace incubator::input
{
    Ec11InputDevice::Ec11InputDevice(
        int pinA,
        int pinB,
        int pinButton)
        :
        m_pinA(pinA),
        m_pinB(pinB),
        m_pinButton(pinButton)
    {
    }

    void Ec11InputDevice::begin()
    {
        pinMode(m_pinA, INPUT_PULLUP);
        pinMode(m_pinB, INPUT_PULLUP);
        pinMode(m_pinButton, INPUT_PULLUP);

        m_lastState = makeState(
            digitalRead(m_pinA) == HIGH,
            digitalRead(m_pinB) == HIGH);

        m_lastRawButtonDown =
            digitalRead(m_pinButton) == LOW;

        m_stableButtonDown = m_lastRawButtonDown;
        m_initialized = true;
    }

    bool Ec11InputDevice::poll(
        uint32_t nowMs,
        InputEvent& event)
    {
        event = InputEvent{};

        if (!m_initialized)
        {
            return false;
        }

        if (pollRotation(nowMs, event))
        {
            return true;
        }

        return pollButton(nowMs, event);
    }

    bool Ec11InputDevice::pollRotation(
        uint32_t nowMs,
        InputEvent& event)
    {
        const uint8_t currentState = makeState(
            digitalRead(m_pinA) == HIGH,
            digitalRead(m_pinB) == HIGH);

        if (currentState == m_lastState)
        {
            return false;
        }

        const uint8_t transition =
            static_cast<uint8_t>((m_lastState << 2) | currentState);

        m_lastState = currentState;

        int8_t direction = 0;

        switch (transition)
        {
            case 0b0001:
            case 0b0111:
            case 0b1110:
            case 0b1000:
                direction = 1;
                break;

            case 0b0010:
            case 0b1011:
            case 0b1101:
            case 0b0100:
                direction = -1;
                break;

            default:
                return false;
        }

        event.source = InputSource::EC11;
        event.timestampMs = nowMs;
        event.delta = direction;
        event.type =
            direction > 0
                ? InputEventType::RotateRight
                : InputEventType::RotateLeft;

        return true;
    }

    bool Ec11InputDevice::pollButton(
        uint32_t nowMs,
        InputEvent& event)
    {
        const bool rawDown =
            digitalRead(m_pinButton) == LOW;

        if (rawDown != m_lastRawButtonDown)
        {
            m_lastRawButtonDown = rawDown;
            m_lastButtonChangeMs = nowMs;
            return false;
        }

        if ((nowMs - m_lastButtonChangeMs) < DebounceMs)
        {
            return false;
        }

        if (rawDown == m_stableButtonDown)
        {
            return false;
        }

        m_stableButtonDown = rawDown;

        event.source = InputSource::EC11;
        event.timestampMs = nowMs;

        if (m_stableButtonDown)
        {
            m_buttonDownStartedMs = nowMs;
            event.type = InputEventType::ButtonDown;
            return true;
        }

        event.type = InputEventType::ButtonClick;
        return true;
    }

    uint8_t Ec11InputDevice::makeState(
        bool a,
        bool b)
    {
        return static_cast<uint8_t>(
            (a ? 0b10 : 0) |
            (b ? 0b01 : 0));
    }
}
