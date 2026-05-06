#include "RuntimeDiagnostics.h"
#include <Arduino.h>

namespace incubator::diagnostics
{
    RuntimeDiagnostics::RuntimeDiagnostics(
        const incubator::domain::RuntimeState& runtime,
        const incubator::performance::PerformanceState& performance)
        : m_runtime(runtime),
          m_performance(performance)
    {
    }

    void RuntimeDiagnostics::tick(uint32_t nowMs)
    {
        if ((nowMs - m_lastPrintMs) < PrintIntervalMs)
        {
            return;
        }

        m_lastPrintMs = nowMs;

        Serial.println();
        Serial.println("[DIAG] Runtime");

        Serial.print("Loop Count: ");
        Serial.println(m_performance.loopCount);

        Serial.print("Last Loop us: ");
        Serial.println(m_performance.lastLoopTimeUs);

        Serial.print("Max Loop us: ");
        Serial.println(m_performance.maxLoopTimeUs);

        Serial.print("Free Heap: ");
        Serial.println(m_performance.freeHeap);

        Serial.print("Min Free Heap: ");
        Serial.println(m_performance.minFreeHeap);

        Serial.print("Overrun: ");
        Serial.println(m_performance.loopOverrun ? "YES" : "NO");

        Serial.print("SafeMode: ");
        Serial.println(m_runtime.safeMode ? "YES" : "NO");

        Serial.print("Sensor: ");
        Serial.println(m_runtime.sensorHealthy ? "OK" : "FAIL");
    }
}
