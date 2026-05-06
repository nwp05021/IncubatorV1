#pragma once

#include <stdint.h>

#include "EventRecord.h"

namespace incubator::event
{
    class EventQueue
    {
    public:
        static constexpr uint16_t Capacity = 32;

    public:
        bool push(
            const EventRecord& event);

        bool pop(
            EventRecord& event);

    private:
        EventRecord m_queue[Capacity];

        uint16_t m_head = 0;

        uint16_t m_tail = 0;

        uint16_t m_count = 0;
    };
}
