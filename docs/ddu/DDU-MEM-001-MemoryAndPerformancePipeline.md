
# DDU-MEM-001 — Memory & Performance Pipeline

> Version: 1.0
> Status: Draft
> Target: ESP32-S3
> Dependency:
> - RuntimeState
> - DiagnosticsService
> - UI Dirty Render
>
> Estimated Time: 20~40 min

---

# 1. 목적

상용 제품 수준의 메모리/성능 감시 구조를 구현한다.

핵심 흐름:

```text
Runtime Loop
    ↓
PerformanceMonitor
    ↓
Runtime Diagnostics
    ↓
System UI / Cloud
```

목표:

- Loop Time 측정
- Heap 상태 측정
- UI Render 부하 감시
- Cloud Publish 부하 감시
- 메모리 누수 조기 감지

---

# 2. 생성 파일

```text
product/performance/PerformanceState.h

product/performance/PerformanceMonitor.h
product/performance/PerformanceMonitor.cpp
```

---

# 3. 핵심 철학

```text
좋은 펌웨어는
느려지기 전에 스스로 감지해야 한다.
```

---

# 4. PerformanceState

## 역할

Runtime 성능 상태 보관.

---

## PerformanceState.h

```cpp
#pragma once

#include <stdint.h>

namespace incubator::performance
{
    struct PerformanceState
    {
        uint32_t loopCount = 0;

        uint32_t lastLoopTimeUs = 0;

        uint32_t maxLoopTimeUs = 0;

        uint32_t freeHeap = 0;

        uint32_t minFreeHeap = 0;

        bool loopOverrun = false;
    };
}
```

---

# 5. PerformanceMonitor

## 역할

```text
Loop Time
Heap
Overrun
```

감시.

---

## PerformanceMonitor.h

```cpp
#pragma once

#include "PerformanceState.h"

namespace incubator::performance
{
    class PerformanceMonitor
    {
    public:
        PerformanceMonitor(
            PerformanceState& state);

    public:
        void beginLoop();

        void endLoop();

        void tick(uint32_t nowMs);

    private:
        PerformanceState& m_state;

        uint32_t m_loopStartUs = 0;

        uint32_t m_lastTickMs = 0;

        static constexpr uint32_t TickIntervalMs =
            5000;

        static constexpr uint32_t LoopOverrunUs =
            20000;
    };
}
```

---

# 6. PerformanceMonitor.cpp

```cpp
#include "PerformanceMonitor.h"
#include <Arduino.h>

namespace incubator::performance
{
    PerformanceMonitor::PerformanceMonitor(
        PerformanceState& state)
        :
        m_state(state)
    {
    }

    void PerformanceMonitor::beginLoop()
    {
        m_loopStartUs = micros();
    }

    void PerformanceMonitor::endLoop()
    {
        const uint32_t elapsed =
            micros() - m_loopStartUs;

        m_state.lastLoopTimeUs =
            elapsed;

        if (elapsed > m_state.maxLoopTimeUs)
        {
            m_state.maxLoopTimeUs =
                elapsed;
        }

        m_state.loopOverrun =
            elapsed > LoopOverrunUs;

        ++m_state.loopCount;
    }

    void PerformanceMonitor::tick(
        uint32_t nowMs)
    {
        if ((nowMs - m_lastTickMs) <
            TickIntervalMs)
        {
            return;
        }

        m_lastTickMs = nowMs;

        m_state.freeHeap =
            ESP.getFreeHeap();

        if (m_state.minFreeHeap == 0 ||
            m_state.freeHeap < m_state.minFreeHeap)
        {
            m_state.minFreeHeap =
                m_state.freeHeap;
        }
    }
}
```

---

# 7. Main Loop 연결

```cpp
void loop()
{
    performance.beginLoop();

    const uint32_t now = millis();

    timeService.tick(now);
    diagnostics.tick(now);
    sensorManager.tick(now);
    scheduler.tick(now);
    climate.tick(now);
    turning.tick(now);
    fan.tick(now);
    alarm.tick(now);
    recovery.tick(now);
    appController.tick();
    eventBuilder.process(runtime, alarmState);
    toast.tick(now);
    ui.tick(now);
    wifi.tick(now);
    aws.tick(now);
    shadow.tick(now);

    performance.tick(now);

    performance.endLoop();
}
```

---

# 8. 성능 기준

| 항목 | 목표 |
|---|---|
| UI 응답 | 100ms 이하 |
| Climate Tick | 500ms 이하 |
| Sensor Poll | 2초 이하 |
| Loop Overrun | 20ms 이상 감지 |

---

# 9. 금지 사항

```text
❌ 반복 new/delete

❌ Runtime vector growth

❌ Blocking network call

❌ Full redraw 남발

❌ delay()
```

---

# 10. Acceptance Criteria

```text
AC-1
Loop time 측정 정상

AC-2
Max loop time 기록

AC-3
Free heap 측정

AC-4
Min free heap 기록

AC-5
Loop overrun 감지
```

---

# 11. 다음 단계

```text
DDU-QA-001
Integration Verification Checklist
```
