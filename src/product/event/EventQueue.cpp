#include "EventQueue.h"

namespace incubator::event
{
    bool EventQueue::push(
        const EventRecord& event)
    {
        if (m_count >= Capacity)
        {
            return false;
        }

        m_queue[m_tail] = event;

        m_tail =
            (m_tail + 1) % Capacity;

        ++m_count;

        return true;
    }

    bool EventQueue::pop(
        EventRecord& event)
    {
        if (m_count == 0)
        {
            return false;
        }

        event = m_queue[m_head];

        m_head =
            (m_head + 1) % Capacity;

        --m_count;

        return true;
    }
}
