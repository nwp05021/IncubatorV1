#include "CmdParser.h"

namespace incubator::cloud
{
    using namespace incubator::app;

    CmdParser::CmdParser(
        CommandQueue& queue)
        :
        m_queue(queue)
    {
    }

    bool CmdParser::processJson(
        const char* json)
    {
        // TODO:
        // StaticJsonDocument Parse

        // Example:
        // cmd = SET_TARGET_TEMP

        return enqueueSetTargetTemp(37.5f);
    }

    bool CmdParser::enqueueSetTargetTemp(
        float value)
    {
        Command cmd;

        cmd.type =
            CommandType::SetTargetTemperature;

        cmd.source =
            CommandSource::Cloud;

        cmd.setFloat.value = value;

        return m_queue.enqueue(cmd);
    }

    bool CmdParser::enqueueSetTargetHumidity(
        float value)
    {
        Command cmd;

        cmd.type =
            CommandType::SetTargetHumidity;

        cmd.source =
            CommandSource::Cloud;

        cmd.setFloat.value =
            value;

        return m_queue.enqueue(cmd);
    }
}