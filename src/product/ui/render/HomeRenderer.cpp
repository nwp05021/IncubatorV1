#include "HomeRenderer.h"

#include <Arduino.h>

namespace incubator::ui
{
    void HomeRenderer::render(
        const HomeUiModel& model)
    {
        renderStatusBar(model);

        renderMainValues(model);

        renderOutputs(model);

        if (model.safeMode)
        {
            Serial.println("[SAFE MODE]");
        }
    }

    void HomeRenderer::renderStatusBar(
        const HomeUiModel& model)
    {
        Serial.print("DAY ");
        Serial.println(model.currentDay);

        Serial.print("WIFI: ");
        Serial.println(
            model.wifiConnected ?
            "OK" :
            "OFF");

        Serial.print("AWS: ");
        Serial.println(
            model.awsConnected ?
            "OK" :
            "OFF");
    }

    void HomeRenderer::renderMainValues(
        const HomeUiModel& model)
    {
        Serial.print("TEMP: ");
        Serial.println(model.tempC);

        Serial.print("HUMIDITY: ");
        Serial.println(model.humidityPct);
    }

    void HomeRenderer::renderOutputs(
        const HomeUiModel& model)
    {
        Serial.print("HEATER: ");
        Serial.println(
            model.heaterOn ?
            "ON" :
            "OFF");

        Serial.print("HUMIDIFIER: ");
        Serial.println(
            model.humidifierOn ?
            "ON" :
            "OFF");
    }
}
