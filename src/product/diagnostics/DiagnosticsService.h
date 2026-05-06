#pragma once

#include "../domain/RuntimeState.h"

namespace incubator::diagnostics
{
    class DiagnosticsService
    {
    public:
        DiagnosticsService(
            incubator::domain::RuntimeState& runtime);

    public:
        void tick(uint32_t nowMs);

        uint32_t getFreeHeap() const;

    private:
        incubator::domain::RuntimeState& m_runtime;

        uint32_t m_lastTickMs = 0;

        static constexpr uint32_t TickIntervalMs =
            5000;
    };
}