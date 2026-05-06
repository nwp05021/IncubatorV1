#pragma once

namespace incubator::devices
{
    class StepperDevice
    {
    public:
        StepperDevice(
            int stepPin,
            int dirPin,
            int enablePin);

    public:
        void begin();

        void stepForward();

        void stepBackward();

        void stop();

    private:
        int m_stepPin;

        int m_dirPin;

        int m_enablePin;
    };
}
