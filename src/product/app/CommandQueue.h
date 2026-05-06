#pragma once

#include "Command.h"

namespace incubator::app
{
    class CommandQueue
    {
    public:
        static constexpr uint16_t Capacity = 16;

    public:
        bool enqueue(const Command& cmd);

        bool dequeue(Command& cmd);

    private:
        Command m_queue[Capacity];

        uint16_t m_head = 0;

        uint16_t m_tail = 0;

        uint16_t m_count = 0;
    };
}
