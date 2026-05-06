#include "AppController.h"

namespace incubator::app
{
    using namespace incubator::domain;

    AppController::AppController(
        RuntimeState& runtime,
        AppSettings& settings,
        CommandQueue& queue)
        :
        m_runtime(runtime),
        m_settings(settings),
        m_queue(queue)
    {
    }

    void AppController::tick()
    {
        Command cmd;

        while (m_queue.dequeue(cmd))
        {
            processCommand(cmd);
        }
    }

    void AppController::processCommand(
        const Command& cmd)
    {
        switch (cmd.type)
        {
            case CommandType::SetTargetTemperature:
                handleSetTargetTemp(cmd);
                break;

            case CommandType::SetTargetHumidity:
                handleSetTargetHumidity(cmd);
                break;

            case CommandType::UiRotateLeft:
                handleUiRotateLeft();
                break;

            case CommandType::UiRotateRight:
                handleUiRotateRight();
                break;

            default:
                break;
        }
    }

    void AppController::handleSetTargetTemp(
        const Command& cmd)
    {
        const float value =
            cmd.setFloat.value;

        if (value < 30.0f ||
            value > 40.0f)
        {
            return;
        }

        m_runtime.targetTempC =
            value;
    }

    void AppController::handleSetTargetHumidity(
        const Command& cmd)
    {
        const float value =
            cmd.setFloat.value;

        if (value < 20.0f ||
            value > 90.0f)
        {
            return;
        }

        m_runtime.targetHumidityPct =
            value;
    }
}

namespace incubator::app
{
    using incubator::domain::UiFocusId;

    void AppController::handleUiRotateLeft()
    {
        m_runtime.focusedItem =
            previousFocus(m_runtime.focusedItem);
    }

    void AppController::handleUiRotateRight()
    {
        m_runtime.focusedItem =
            nextFocus(m_runtime.focusedItem);
    }

    UiFocusId AppController::nextFocus(
        UiFocusId current) const
    {
        switch (current)
        {
            case UiFocusId::Temperature:
                return UiFocusId::Humidity;

            case UiFocusId::Humidity:
                return UiFocusId::Fan;

            case UiFocusId::Fan:
                return UiFocusId::StartButton;

            case UiFocusId::StartButton:
                return UiFocusId::StopButton;

            case UiFocusId::StopButton:
                return UiFocusId::Temperature;

            default:
                return UiFocusId::Temperature;
        }
    }

    UiFocusId AppController::previousFocus(
        UiFocusId current) const
    {
        switch (current)
        {
            case UiFocusId::Temperature:
                return UiFocusId::StopButton;

            case UiFocusId::Humidity:
                return UiFocusId::Temperature;

            case UiFocusId::Fan:
                return UiFocusId::Humidity;

            case UiFocusId::StartButton:
                return UiFocusId::Fan;

            case UiFocusId::StopButton:
                return UiFocusId::StartButton;

            default:
                return UiFocusId::Temperature;
        }
    }
}
