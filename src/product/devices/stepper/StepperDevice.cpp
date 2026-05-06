#include "StepperDevice.h"
#include <Arduino.h>

namespace incubator::devices
{
    StepperDevice::StepperDevice(
        int stepPin,
        int dirPin,
        int enablePin)
        :
        m_stepPin(stepPin),
        m_dirPin(dirPin),
        m_enablePin(enablePin)
    {
    }

    void StepperDevice::begin()
    {
        pinMode(m_stepPin, OUTPUT);
        pinMode(m_dirPin, OUTPUT);
        pinMode(m_enablePin, OUTPUT);

        stop();
    }

    void StepperDevice::stepForward()
    {
        digitalWrite(m_enablePin, LOW);
        digitalWrite(m_dirPin, HIGH);
        digitalWrite(m_stepPin, HIGH);
        digitalWrite(m_stepPin, LOW);
    }

    void StepperDevice::stepBackward()
    {
        digitalWrite(m_enablePin, LOW);
        digitalWrite(m_dirPin, LOW);
        digitalWrite(m_stepPin, HIGH);
        digitalWrite(m_stepPin, LOW);
    }

    void StepperDevice::stop()
    {
        digitalWrite(m_enablePin, HIGH);
    }
}
