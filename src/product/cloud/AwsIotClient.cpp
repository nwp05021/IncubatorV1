#include "AwsIotClient.h"

#include <Arduino.h>

namespace incubator::cloud
{
    void AwsIotClient::begin()
    {
        m_connected = true;
    }

    void AwsIotClient::tick()
    {
    }

    void AwsIotClient::publishTelemetry(
        const incubator::domain::RuntimeState& runtime)
    {
        Serial.print("TEMP=");
        Serial.println(runtime.currentTempC);

        Serial.print("HUMIDITY=");
        Serial.println(runtime.currentHumidityPct);
    }

    bool AwsIotClient::connected() const
    {
        return m_connected;
    }
}
