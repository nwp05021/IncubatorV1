#include "CommandQueue.h"

namespace incubator::app
{
    bool CommandQueue::enqueue(
        const Command& cmd)
    {
        if (m_count >= Capacity)
        {
            return false;
        }

        m_queue[m_tail] = cmd;

        m_tail =
            (m_tail + 1) % Capacity;

        ++m_count;

        return true;
    }

    bool CommandQueue::dequeue(
        Command& cmd)
    {
        if (m_count == 0)
        {
            return false;
        }

        cmd = m_queue[m_head];

        m_head =
            (m_head + 1) % Capacity;

        --m_count;

        return true;
    }
}
