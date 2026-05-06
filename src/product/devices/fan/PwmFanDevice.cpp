#include "PwmFanDevice.h"
#include <Arduino.h>

namespace incubator::devices
{
    PwmFanDevice::PwmFanDevice(
        int pin,
        int channel)
        :
        m_pin(pin),
        m_channel(channel)
    {
    }

    void PwmFanDevice::begin()
    {
        ledcSetup(
            m_channel,
            25000,
            8);

        ledcAttachPin(
            m_pin,
            m_channel);

        setDuty(0);
    }

    void PwmFanDevice::setDuty(
        uint8_t dutyPct)
    {
        if (dutyPct > 100)
        {
            dutyPct = 100;
        }

        const uint32_t duty =
            (dutyPct * 255) / 100;

        ledcWrite(
            m_channel,
            duty);
    }
}
