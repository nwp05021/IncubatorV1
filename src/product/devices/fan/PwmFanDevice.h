#pragma once

#include <stdint.h>

namespace incubator::devices
{
    class PwmFanDevice
    {
    public:
        PwmFanDevice(
            int pin,
            int channel);

    public:
        void begin();

        void setDuty(
            uint8_t dutyPct);

    private:
        int m_pin;

        int m_channel;
    };
}
