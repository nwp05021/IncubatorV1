#pragma once

#include <stdint.h>
#include "InputEvent.h"

namespace incubator::input
{
    class InputEventQueue
    {
    public:
        static constexpr uint16_t Capacity = 16;

    public:
        bool enqueue(const InputEvent& event);
        bool dequeue(InputEvent& event);
        bool isEmpty() const;
        uint16_t count() const;

    private:
        InputEvent m_queue[Capacity];
        uint16_t m_head = 0;
        uint16_t m_tail = 0;
        uint16_t m_count = 0;
    };
}
