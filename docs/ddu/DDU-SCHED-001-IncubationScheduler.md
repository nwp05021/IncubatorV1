
# DDU-SCHED-001 — Incubation Scheduler

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + PlatformIO
> Dependency:
> - RuntimeState
> - AppSettings
> - IncubationBatch
> - IncubationPlanTable
>
> Estimated Time: 20~40 min

---

# 1. 목적

부화 Day 기반 정책 자동 적용.

핵심 흐름:

```text
Batch
    ↓
Current Day
    ↓
Plan Row
    ↓
RuntimeState Target
```

목표:

- 현재 Day 자동 계산
- Plan Row 자동 적용
- Lockdown 자동 판단
- RuntimeState Target 자동 갱신

---

# 2. 생성 파일

```text
product/domain/IncubationBatch.h
product/domain/PlanRow.h
product/domain/IncubationPlanTable.h

product/modules/scheduler/SchedulerModule.h
product/modules/scheduler/SchedulerModule.cpp
```

---

# 3. 핵심 철학

Scheduler는:

```text
현재 어떤 목표값을 적용해야 하는가
```

만 계산한다.

---

## 금지

```text
❌ GPIO 제어
❌ UI 접근
❌ 저장 처리
❌ Cloud 처리
```

---

# 4. IncubationBatch

## 역할

현재 진행 중인 부화 세션.

---

## IncubationBatch.h

```cpp
#pragma once

#include <stdint.h>

namespace incubator::domain
{
    struct IncubationBatch
    {
        bool active = false;

        uint32_t startEpoch = 0;

        uint16_t totalDays = 21;

        uint16_t lockdownStartDay = 19;

        char batchId[32] = {0};
    };
}
```

---

# 5. PlanRow

## 역할

특정 Day 목표값.

---

## PlanRow.h

```cpp
#pragma once

#include <stdint.h>

namespace incubator::domain
{
    struct PlanRow
    {
        uint16_t day = 0;

        float targetTempC = 37.5f;

        float targetHumidityPct = 60.0f;

        bool turningEnabled = true;

        uint32_t turningIntervalMs = 7200000;
    };
}
```

---

# 6. IncubationPlanTable

## 역할

Day별 정책 테이블.

---

## IncubationPlanTable.h

```cpp
#pragma once

#include "PlanRow.h"

namespace incubator::domain
{
    class IncubationPlanTable
    {
    public:
        static constexpr uint16_t MaxRows = 32;

    public:
        bool getRow(
            uint16_t day,
            PlanRow& outRow) const;

    public:
        PlanRow rows[MaxRows];

        uint16_t rowCount = 0;
    };
}
```

---

# 7. SchedulerModule

## 역할

```text
Batch
    ↓
Current Day 계산
    ↓
Plan 적용
    ↓
RuntimeState Target 갱신
```

---

## SchedulerModule.h

```cpp
#pragma once

#include "../../domain/RuntimeState.h"
#include "../../domain/IncubationBatch.h"
#include "../../domain/IncubationPlanTable.h"

namespace incubator::modules::scheduler
{
    class SchedulerModule
    {
    public:
        SchedulerModule(
            incubator::domain::RuntimeState& runtime,
            incubator::domain::IncubationBatch& batch,
            incubator::domain::IncubationPlanTable& plan);

    public:
        void tick(uint32_t nowMs);

    private:
        void updateCurrentDay(uint32_t nowMs);

        void applyPlan();

    private:
        incubator::domain::RuntimeState& m_runtime;

        incubator::domain::IncubationBatch& m_batch;

        incubator::domain::IncubationPlanTable& m_plan;

        uint32_t m_lastTickMs = 0;

        static constexpr uint32_t TickIntervalMs = 10000;
    };
}
```

---

# 8. SchedulerModule.cpp

```cpp
#include "SchedulerModule.h"

namespace incubator::modules::scheduler
{
    using namespace incubator::domain;

    SchedulerModule::SchedulerModule(
        RuntimeState& runtime,
        IncubationBatch& batch,
        IncubationPlanTable& plan)
        :
        m_runtime(runtime),
        m_batch(batch),
        m_plan(plan)
    {
    }

    void SchedulerModule::tick(uint32_t nowMs)
    {
        if ((nowMs - m_lastTickMs) < TickIntervalMs)
        {
            return;
        }

        m_lastTickMs = nowMs;

        if (!m_batch.active)
        {
            return;
        }

        updateCurrentDay(nowMs);

        applyPlan();
    }

    void SchedulerModule::updateCurrentDay(uint32_t nowMs)
    {
        const uint32_t elapsedSec =
            (nowMs / 1000);

        const uint32_t elapsedDays =
            elapsedSec / 86400;

        m_runtime.currentDay =
            static_cast<uint16_t>(elapsedDays + 1);

        m_runtime.totalDays =
            m_batch.totalDays;

        m_runtime.lockdown =
            (m_runtime.currentDay >=
             m_batch.lockdownStartDay);
    }

    void SchedulerModule::applyPlan()
    {
        PlanRow row;

        if (!m_plan.getRow(
                m_runtime.currentDay,
                row))
        {
            return;
        }

        m_runtime.targetTempC =
            row.targetTempC;

        m_runtime.targetHumidityPct =
            row.targetHumidityPct;
    }
}
```

---

# 9. Main Loop 연결

## main.cpp

```cpp
void loop()
{
    const uint32_t now = millis();

    sensorManager.tick(now);

    scheduler.tick(now);

    climate.tick(now);

    appController.tick();

    ui.tick(now);

    cloud.tick(now);
}
```

---

# 10. Lockdown 전략

## 핵심 원칙

```text
Lockdown 시 전란 중단
```

---

## Lockdown 조건

```text
currentDay >= lockdownStartDay
```

---

# 11. RuntimeState 반영

Scheduler는:

```text
targetTempC
targetHumidityPct
currentDay
lockdown
```

를 RuntimeState에 기록한다.

---

# 12. 핵심 장점

## 1) Day 기반 자동 제어

```text
사용자 개입 최소화
```

---

## 2) RuntimeState 중심 구조

모든 화면/Cloud 동일 데이터 사용.

---

## 3) Plan 수정 단순화

```text
Plan 수정
    ↓
Scheduler 적용
```

구조 유지.

---

# 13. 금지 사항

```text
❌ Scheduler 내부 Relay 제어

❌ Scheduler 내부 Alarm 발생

❌ Plan 내부 동적 메모리 증가

❌ RuntimeState direct mutation from UI
```

---

# 14. Acceptance Criteria

```text
AC-1
10초 Tick 기반 동작

AC-2
currentDay 자동 계산

AC-3
Lockdown 자동 판단

AC-4
Plan Row 정상 적용

AC-5
RuntimeState Target 자동 갱신
```

---

# 15. 다음 단계

다음 DDU:

```text
DDU-ALARM-001
Alarm Pipeline
```

다음 구현 예정:

- HighTemp Alarm
- LowTemp Alarm
- SensorFail Alarm
- Alarm Delay
- Alarm Overlay Trigger
- SafeMode Trigger
