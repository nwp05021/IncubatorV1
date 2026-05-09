#pragma once
#include "domain/RuntimeState.h"
#include "domain/IncubationBatch.h"
#include <cstddef>

namespace incubator::cloud
{
    class TelemetryBuilder
    {
    public:
        static size_t build(const domain::RuntimeState& state,
                            const domain::IncubationBatch& batch,
                            char* buf, size_t bufSize);
    };
}
