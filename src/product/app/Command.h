#pragma once

#include <stdint.h>

namespace incubator::app
{
    enum class CommandType
    {
        None,

        SetTargetTemperature,
        SetTargetHumidity,

        HeaterOn,
        HeaterOff,

        FanPwm,

        EnterSafeMode,
        ClearSafeMode
    };

    enum class CommandSource
    {
        UI,
        Cloud,
        BLE,
        Recovery,
        Internal
    };

    struct FloatPayload
    {
        float value = 0.0f;
    };

    struct UIntPayload
    {
        uint32_t value = 0;
    };

    struct Command
    {
        CommandType type =
            CommandType::None;

        CommandSource source =
            CommandSource::Internal;

        union
        {
            FloatPayload setFloat;

            UIntPayload setUInt;
        };

        Command()
        {
            setFloat.value = 0.0f;
        }
    };
}