#pragma once

namespace incubator::event
{
    enum class EventLevel
    {
        Info,
        Warning,
        Alarm,
        Critical
    };

    struct EventRecord
    {
        EventLevel level =
            EventLevel::Info;

        char message[64] = {0};
    };
}
