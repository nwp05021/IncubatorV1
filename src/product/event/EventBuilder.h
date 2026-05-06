#pragma once

#include "../domain/RuntimeState.h"
#include "../domain/AlarmState.h"

#include "EventQueue.h"

namespace incubator::event
{
    class EventBuilder
    {
    public:
        EventBuilder(
            EventQueue& queue);

    public:
        void process(
            const incubator::domain::RuntimeState& runtime,
            const incubator::domain::AlarmState& alarm);

    private:
        void pushAlarmEvent(
            const char* message);

    private:
        EventQueue& m_queue;

        bool m_prevHighTemp = false;

        bool m_prevSafeMode = false;
    };
}