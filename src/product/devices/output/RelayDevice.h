#pragma once

namespace incubator::devices
{
    class RelayDevice
    {
    public:
        RelayDevice(int pin);

    public:
        void begin();

        void on();

        void off();

    private:
        int m_pin;
    };
}
