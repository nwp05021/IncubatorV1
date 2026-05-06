#include "GraphicHomeRenderer.h"

#include "../theme/UiTheme.h"

#include "../../domain/UiFocusId.h"

namespace incubator::ui
{
    GraphicHomeRenderer::GraphicHomeRenderer(
        incubator::devices::IDisplayDevice& display)
        :
        m_display(display),
        m_statusBarWidget(display),
        m_metricCardWidget(display),
        m_outputBarWidget(display),
        m_progressWidget(display)
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

        const bool spriteFrameStarted =
            m_display.beginSpriteFrame(UiTheme::Background);

        if (spriteFrameStarted)
        {
            renderStatusBar(model);
            renderTemperature(model);
            renderHumidity(model);
            renderOutputs(model);
            renderProgress(model);
            renderOverlay(model);

            m_display.endSpriteFrame();
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
        m_statusBarWidget.render(
            m_layout,
            model);
    }

    void GraphicHomeRenderer::renderTemperature(
        const HomeUiModel& model)
    {
        MetricCardModel card;
        card.title = "TEMP";
        card.value = model.tempC;
        card.decimals = 1;
        card.unit = "C";
        card.statusText = model.heaterOn ? "HEATER ON" : "HEATER OFF";
        card.accentColor = UiTheme::Accent;
        card.statusColor = model.heaterOn ? UiTheme::Warning : UiTheme::MutedText;
        card.focused =
            model.focusedItem == incubator::domain::UiFocusId::Temperature;

        m_metricCardWidget.render(
            m_layout.tempCard,
            card);
    }

    void GraphicHomeRenderer::renderHumidity(
        const HomeUiModel& model)
    {
        MetricCardModel card;
        card.title = "HUMIDITY";
        card.value = model.humidityPct;
        card.decimals = 0;
        card.unit = "%";
        card.statusText = model.humidifierOn ? "HUMID ON" : "HUMID OFF";
        card.accentColor = UiTheme::Accent;
        card.statusColor = model.humidifierOn ? UiTheme::Accent : UiTheme::MutedText;
        card.focused =
            model.focusedItem == incubator::domain::UiFocusId::Humidity;

        m_metricCardWidget.render(
            m_layout.humidityCard,
            card);
    }

    void GraphicHomeRenderer::renderOutputs(
        const HomeUiModel& model)
    {
        m_outputBarWidget.render(
            m_layout.outputBar,
            model);
    }

    void GraphicHomeRenderer::renderProgress(
        const HomeUiModel& model)
    {
        m_progressWidget.render(
            m_layout,
            model);
    }

    void GraphicHomeRenderer::renderOverlay(
        const HomeUiModel& model)
    {
        if (model.safeMode)
        {
            m_display.fillRect(
                m_layout.overlay.x,
                m_layout.overlay.y,
                m_layout.overlay.w,
                m_layout.overlay.h,
                UiTheme::Danger);

            m_display.drawText(
                m_layout.overlay.x + 40,
                m_layout.overlay.y + 34,
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
