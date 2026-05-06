#include "GraphicHomeRenderer.h"

#include "../theme/UiTheme.h"

namespace incubator::ui
{
    GraphicHomeRenderer::GraphicHomeRenderer(
        incubator::devices::IDisplayDevice& display)
        :
        m_display(display)
    {
    }

    void GraphicHomeRenderer::render(
        const HomeUiModel& model,
        const HomeDirtyFlags& dirty)
    {
        if (!dirty.any())
        {
            return;
        }

        m_display.beginFrame();

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

        m_display.endFrame();
    }

    void GraphicHomeRenderer::renderStatusBar(
        const HomeUiModel& model)
    {
        m_display.fillRect(
            0,
            0,
            320,
            28,
            UiTheme::Surface);

        m_display.drawText(
            8,
            7,
            "INCUBATOR",
            UiTheme::Text,
            UiTheme::Surface,
            2);

        m_display.drawText(
            190,
            7,
            model.wifiConnected ? "WIFI" : "NO WIFI",
            model.wifiConnected ? UiTheme::Good : UiTheme::Warning,
            UiTheme::Surface,
            1);

        m_display.drawText(
            260,
            7,
            model.awsConnected ? "AWS" : "OFF",
            model.awsConnected ? UiTheme::Good : UiTheme::MutedText,
            UiTheme::Surface,
            1);
    }

    void GraphicHomeRenderer::renderTemperature(
        const HomeUiModel& model)
    {
        m_display.fillRect(
            0,
            32,
            156,
            108,
            UiTheme::Surface2);

        m_display.drawText(
            10,
            42,
            "TEMP",
            UiTheme::MutedText,
            UiTheme::Surface2,
            1);

        m_display.drawFloat(
            10,
            66,
            model.tempC,
            1,
            UiTheme::Text,
            UiTheme::Surface2,
            4);

        m_display.drawText(
            112,
            82,
            "C",
            UiTheme::Accent,
            UiTheme::Surface2,
            2);

        m_display.drawText(
            10,
            120,
            model.heaterOn ? "HEATER ON" : "HEATER OFF",
            model.heaterOn ? UiTheme::Warning : UiTheme::MutedText,
            UiTheme::Surface2,
            1);
    }

    void GraphicHomeRenderer::renderHumidity(
        const HomeUiModel& model)
    {
        m_display.fillRect(
            164,
            32,
            156,
            108,
            UiTheme::Surface2);

        m_display.drawText(
            174,
            42,
            "HUMIDITY",
            UiTheme::MutedText,
            UiTheme::Surface2,
            1);

        m_display.drawFloat(
            174,
            66,
            model.humidityPct,
            0,
            UiTheme::Text,
            UiTheme::Surface2,
            4);

        m_display.drawText(
            276,
            82,
            "%",
            UiTheme::Accent,
            UiTheme::Surface2,
            2);

        m_display.drawText(
            174,
            120,
            model.humidifierOn ? "HUMID ON" : "HUMID OFF",
            model.humidifierOn ? UiTheme::Accent : UiTheme::MutedText,
            UiTheme::Surface2,
            1);
    }

    void GraphicHomeRenderer::renderOutputs(
        const HomeUiModel& model)
    {
        m_display.fillRect(
            0,
            146,
            320,
            38,
            UiTheme::Surface);

        m_display.drawText(
            8,
            158,
            "OUTPUT",
            UiTheme::MutedText,
            UiTheme::Surface,
            1);

        m_display.drawText(
            80,
            158,
            model.heaterOn ? "HEAT" : "----",
            model.heaterOn ? UiTheme::Warning : UiTheme::MutedText,
            UiTheme::Surface,
            1);

        m_display.drawText(
            140,
            158,
            model.humidifierOn ? "HUMI" : "----",
            model.humidifierOn ? UiTheme::Accent : UiTheme::MutedText,
            UiTheme::Surface,
            1);
    }

    void GraphicHomeRenderer::renderProgress(
        const HomeUiModel& model)
    {
        m_display.fillRect(
            0,
            190,
            320,
            50,
            UiTheme::Background);

        int progress = 0;

        if (model.totalDays > 0)
        {
            progress =
                (model.currentDay * 100) /
                model.totalDays;
        }

        m_display.drawText(
            8,
            196,
            "INCUBATION PROGRESS",
            UiTheme::MutedText,
            UiTheme::Background,
            1);

        m_display.drawProgressBar(
            8,
            218,
            304,
            14,
            progress,
            UiTheme::Accent,
            UiTheme::Surface);
    }

    void GraphicHomeRenderer::renderOverlay(
        const HomeUiModel& model)
    {
        if (model.safeMode)
        {
            m_display.fillRect(
                36,
                70,
                248,
                100,
                UiTheme::Danger);

            m_display.drawText(
                76,
                104,
                "SAFE MODE",
                UiTheme::Text,
                UiTheme::Danger,
                3);

            return;
        }

        if (model.highTempAlarm)
        {
            m_display.fillRect(
                40,
                76,
                240,
                80,
                UiTheme::Danger);

            m_display.drawText(
                72,
                104,
                "HIGH TEMP",
                UiTheme::Text,
                UiTheme::Danger,
                3);

            return;
        }

        if (model.lowTempAlarm)
        {
            m_display.fillRect(
                40,
                76,
                240,
                80,
                UiTheme::Warning);

            m_display.drawText(
                82,
                104,
                "LOW TEMP",
                UiTheme::Background,
                UiTheme::Warning,
                3);

            return;
        }
    }
}
