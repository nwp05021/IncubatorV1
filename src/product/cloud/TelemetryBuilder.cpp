#include <stddef.h>
#include "TelemetryBuilder.h"

namespace incubator::cloud
{
    using namespace incubator::domain;

    void TelemetryBuilder::build(
        const RuntimeState& runtime,
        TelemetryPayload& payload)
    {
        payload.currentTempC =
            runtime.currentTempC;

        payload.currentHumidityPct =
            runtime.currentHumidityPct;

        payload.targetTempC =
            runtime.targetTempC;

        payload.targetHumidityPct =
            runtime.targetHumidityPct;

        payload.heaterOn =
            runtime.heaterOn;

        payload.humidifierOn =
            runtime.humidifierOn;

        payload.safeMode =
            runtime.safeMode;

        payload.currentDay =
            runtime.currentDay;

        payload.wifiConnected =
            runtime.wifiConnected;

        payload.awsConnected =
            runtime.awsConnected;
    }

    bool TelemetryBuilder::serialize(
        const TelemetryPayload& payload,
        char* buffer,
        size_t size)
    {
        // TODO:
        // StaticJsonDocument Serialize

        return true;
    }
}