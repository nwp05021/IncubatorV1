#include "RelayDevice.h"
#include <Arduino.h>

namespace incubator::devices
{
    RelayDevice::RelayDevice(int pin)
        :
        m_pin(pin)
    {
    }

    void RelayDevice::begin()
    {
        pinMode(m_pin, OUTPUT);

        off();
    }

    void RelayDevice::on()
    {
        digitalWrite(m_pin, HIGH);
    }

    void RelayDevice::off()
    {
        digitalWrite(m_pin, LOW);
    }
}
