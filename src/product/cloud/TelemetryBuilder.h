#pragma once

#include <stddef.h>

#include "../domain/RuntimeState.h"
#include "TelemetryPayload.h"

namespace incubator::cloud
{
    class TelemetryBuilder
    {
    public:
        void build(
            const incubator::domain::RuntimeState& runtime,
            TelemetryPayload& payload);

        bool serialize(
            const TelemetryPayload& payload,
            char* buffer,
            size_t size);
    };
}