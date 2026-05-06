#pragma once

#include <stdint.h>
#include "IEc11Device.h"

namespace incubator::input
{
    class Ec11InputDevice final : public IEc11Device
    {
    public:
        Ec11InputDevice(
            int pinA,
            int pinB,
            int pinButton);

    public:
        void begin() override;

        bool poll(
            uint32_t nowMs,
            InputEvent& event) override;

    private:
        bool pollRotation(
            uint32_t nowMs,
            InputEvent& event);

        bool pollButton(
            uint32_t nowMs,
            InputEvent& event);

        static uint8_t makeState(
            bool a,
            bool b);

    private:
        int m_pinA;
        int m_pinB;
        int m_pinButton;

        uint8_t m_lastState = 0;
        bool m_initialized = false;

        bool m_lastRawButtonDown = false;
        bool m_stableButtonDown = false;
        uint32_t m_lastButtonChangeMs = 0;
        uint32_t m_buttonDownStartedMs = 0;

        static constexpr uint32_t DebounceMs = 35;
    };
}
