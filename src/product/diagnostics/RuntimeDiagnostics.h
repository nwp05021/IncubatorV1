#pragma once

#include <stdint.h>
#include "../domain/RuntimeState.h"
#include "../performance/PerformanceState.h"

namespace incubator::diagnostics
{
    class RuntimeDiagnostics
    {
    public:
        RuntimeDiagnostics(
            const incubator::domain::RuntimeState& runtime,
            const incubator::performance::PerformanceState& performance);

        void tick(uint32_t nowMs);

    private:
        const incubator::domain::RuntimeState& m_runtime;
        const incubator::performance::PerformanceState& m_performance;

        uint32_t m_lastPrintMs = 0;

        static constexpr uint32_t PrintIntervalMs = 5000;
    };
}
