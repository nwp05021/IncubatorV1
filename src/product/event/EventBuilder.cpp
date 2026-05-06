#include "EventBuilder.h"

#include <string.h>

namespace incubator::event
{
    using namespace incubator::domain;

    EventBuilder::EventBuilder(
        EventQueue& queue)
        :
        m_queue(queue)
    {
    }

    void EventBuilder::process(
        const RuntimeState& runtime,
        const AlarmState& alarm)
    {
        if (alarm.highTemp &&
            !m_prevHighTemp)
        {
            pushAlarmEvent(
                "HIGH TEMP");
        }

        if (runtime.safeMode &&
            !m_prevSafeMode)
        {
            pushAlarmEvent(
                "SAFE MODE");
        }

        m_prevHighTemp =
            alarm.highTemp;

        m_prevSafeMode =
            runtime.safeMode;
    }

    void EventBuilder::pushAlarmEvent(
        const char* message)
    {
        EventRecord event;

        event.level =
            EventLevel::Alarm;

        strncpy(
            event.message,
            message,
            sizeof(event.message) - 1);

        m_queue.push(event);
    }
}