#include "InputEventQueue.h"

namespace incubator::input
{
    bool InputEventQueue::enqueue(const InputEvent& event)
    {
        if (m_count >= Capacity)
        {
            return false;
        }

        m_queue[m_tail] = event;
        m_tail = (m_tail + 1) % Capacity;
        ++m_count;

        return true;
    }

    bool InputEventQueue::dequeue(InputEvent& event)
    {
        if (m_count == 0)
        {
            return false;
        }

        event = m_queue[m_head];
        m_head = (m_head + 1) % Capacity;
        --m_count;

        return true;
    }

    bool InputEventQueue::isEmpty() const
    {
        return m_count == 0;
    }

    uint16_t InputEventQueue::count() const
    {
        return m_count;
    }
}
