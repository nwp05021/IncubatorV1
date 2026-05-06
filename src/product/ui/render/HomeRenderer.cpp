#include "HomeRenderer.h"

#include <Arduino.h>

namespace incubator::ui
{
    void HomeRenderer::render(
        const HomeUiModel& model,
        const HomeDirtyFlags& dirty)
    {
        if (!dirty.any())
        {
            return;
        }

        if (dirty.statusBar)
        {
            renderStatusBar(model);
        }

        if (dirty.temperature)
        {
            renderTemperature(model);
        }

        if (dirty.humidity)
        {
            renderHumidity(model);
        }

        if (dirty.outputs)
        {
            renderOutputs(model);
        }

        if (dirty.progress)
        {
            renderProgress(model);
        }

        if (dirty.overlay)
        {
            renderOverlay(model);
        }
    }

    void HomeRenderer::renderStatusBar(
        const HomeUiModel& model)
    {
        Serial.print("[UI] Status WiFi=");
        Serial.print(model.wifiConnected ? "OK" : "OFF");
        Serial.print(" AWS=");
        Serial.print(model.awsConnected ? "OK" : "OFF");
        Serial.print(" Safe=");
        Serial.println(model.safeMode ? "YES" : "NO");
    }

    void HomeRenderer::renderTemperature(
        const HomeUiModel& model)
    {
        Serial.print("[UI] Temp ");
        Serial.print(model.tempC);
        Serial.print(" Heater=");
        Serial.println(model.heaterOn ? "ON" : "OFF");
    }

    void HomeRenderer::renderHumidity(
        const HomeUiModel& model)
    {
        Serial.print("[UI] Humidity ");
        Serial.print(model.humidityPct);
        Serial.print(" Humidifier=");
        Serial.println(model.humidifierOn ? "ON" : "OFF");
    }

    void HomeRenderer::renderOutputs(
        const HomeUiModel& model)
    {
        Serial.print("[UI] Outputs H=");
        Serial.print(model.heaterOn ? "ON" : "OFF");
        Serial.print(" HU=");
        Serial.print(model.humidifierOn ? "ON" : "OFF");
        Serial.print(" Fan=");
        Serial.print(model.fanPwm);
        Serial.println("%");
    }

    void HomeRenderer::renderProgress(
        const HomeUiModel& model)
    {
        Serial.print("[UI] Day ");
        Serial.print(model.currentDay);
        Serial.print("/");
        Serial.println(model.totalDays);
    }

    void HomeRenderer::renderOverlay(
        const HomeUiModel& model)
    {
        if (model.safeMode)
        {
            Serial.println("[UI] OVERLAY SAFE MODE");
            return;
        }

        if (model.highTempAlarm)
        {
            Serial.println("[UI] OVERLAY HIGH TEMP");
            return;
        }

        if (model.lowTempAlarm)
        {
            Serial.println("[UI] OVERLAY LOW TEMP");
            return;
        }
    }
}
