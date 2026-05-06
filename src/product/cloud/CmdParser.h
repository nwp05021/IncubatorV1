#pragma once

#include "../app/CommandQueue.h"

namespace incubator::cloud
{
    class CmdParser
    {
    public:
        CmdParser(
            incubator::app::CommandQueue& queue);

    public:
        bool processJson(
            const char* json);

    private:
        bool enqueueSetTargetTemp(
            float value);

        bool enqueueSetTargetHumidity(
            float value);

    private:
        incubator::app::CommandQueue& m_queue;
    };
}