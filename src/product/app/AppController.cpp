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