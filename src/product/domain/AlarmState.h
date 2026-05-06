#pragma once

namespace incubator::domain
{
    struct AlarmState
    {
        bool highTemp = false;

        bool lowTemp = false;

        bool sensorFail = false;
    };
}
