#ifdef INCUBATOR_ENABLE_CLOUD
#include "cloud/TelemetryBuilder.h"
#include <ArduinoJson.h>

namespace incubator::cloud
{

size_t TelemetryBuilder::build(const domain::RuntimeState& state,
                               const domain::IncubationBatch& batch,
                               char* buf, size_t bufSize)
{
    DynamicJsonDocument doc(2048);
    doc["deviceId"] = batch.batchId[0] ? batch.batchId : "";
    doc["batchId"] = batch.batchId[0] ? batch.batchId : "";
    doc["ts"] = static_cast<uint32_t>(time(nullptr));
    doc["day"] = state.currentDay;

    auto sensor = doc.createNestedObject("sensor");
    sensor["tempC"] = state.currentTempC;
    sensor["humidityPct"] = state.currentHumidityPct;
    sensor["tempOk"] = state.tempSensorOk;
    sensor["humiOk"] = state.humiSensorOk;

    auto actuator = doc.createNestedObject("actuator");
    actuator["heater"] = state.heaterOn;
    actuator["humidifier"] = state.humidifierOn;
    actuator["turner"] = state.turnerOn;
    actuator["fanDuty"] = 0;

    auto target = doc.createNestedObject("target");
    target["tempC"] = state.targetTempC;
    target["humidityPct"] = state.targetHumidityPct;

    auto alarm = doc.createNestedObject("alarm");
    alarm["temp"] = state.tempAlarmActive;
    alarm["humi"] = state.humiAlarmActive;

    auto progress = doc.createNestedObject("progress");
    progress["currentDay"] = state.currentDay;
    progress["totalDays"] = state.totalDays;
    progress["lockdown"] = state.lockdownActive;

    return serializeJson(doc, buf, bufSize);
}

} // namespace incubator::cloud
#endif
