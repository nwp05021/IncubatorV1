
# DDU-REC-001 — Recovery & SafeMode Manager

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + PlatformIO
> Dependency:
> - RuntimeState
> - AppSettings
> - Alarm Pipeline
> - Scheduler
>
> Estimated Time: 20~40 min

---

# 1. 목적

부팅 복구 및 SafeMode 관리 구조 구현.

핵심 흐름:

```text
Boot
    ↓
Load Settings
    ↓
Load Batch
    ↓
Load Plan
    ↓
Validation
    ↓
SafeMode 판단
```

목표:

- 부팅 복구
- WDT 복구
- SafeMode 진입
- 출력 차단
- Factory Reset 준비

---

# 2. 생성 파일

```text
product/domain/RecoveryState.h

product/recovery/RecoveryManager.h
product/recovery/RecoveryManager.cpp
```

---

# 3. 핵심 철학

```text
복구보다 중요한 것은 안전 정지다.
```

---

# 4. RecoveryState

## 역할

복구 진단 정보 저장.

---

## RecoveryState.h

```cpp
#pragma once

#include <stdint.h>

namespace incubator::domain
{
    enum class SafeModeReason
    {
        None,
        SensorFail,
        PlanCorrupt,
        StorageFail,
        WdtReset,
        CriticalAlarm
    };

    struct RecoveryState
    {
        uint32_t bootCount = 0;

        uint32_t wdtResetCount = 0;

        bool lastBootRecovered = false;

        SafeModeReason safeModeReason =
            SafeModeReason::None;
    };
}
```

---

# 5. RecoveryManager

## 역할

```text
복구 상태 판단
SafeMode 진입
출력 차단
```

---

## RecoveryManager.h

```cpp
#pragma once

#include "../domain/RuntimeState.h"
#include "../domain/RecoveryState.h"

namespace incubator::recovery
{
    class RecoveryManager
    {
    public:
        RecoveryManager(
            incubator::domain::RuntimeState& runtime,
            incubator::domain::RecoveryState& recovery);

    public:
        void boot();

        void tick(uint32_t nowMs);

    private:
        void evaluateSafeMode();

        void enterSafeMode(
            incubator::domain::SafeModeReason reason);

    private:
        incubator::domain::RuntimeState& m_runtime;

        incubator::domain::RecoveryState& m_recovery;

        uint32_t m_lastTickMs = 0;

        static constexpr uint32_t TickIntervalMs = 1000;
    };
}
```

---

# 6. RecoveryManager.cpp

```cpp
#include "RecoveryManager.h"

namespace incubator::recovery
{
    using namespace incubator::domain;

    RecoveryManager::RecoveryManager(
        RuntimeState& runtime,
        RecoveryState& recovery)
        :
        m_runtime(runtime),
        m_recovery(recovery)
    {
    }

    void RecoveryManager::boot()
    {
        ++m_recovery.bootCount;

        evaluateSafeMode();
    }

    void RecoveryManager::tick(uint32_t nowMs)
    {
        if ((nowMs - m_lastTickMs) < TickIntervalMs)
        {
            return;
        }

        m_lastTickMs = nowMs;

        evaluateSafeMode();
    }

    void RecoveryManager::evaluateSafeMode()
    {
        if (!m_runtime.sensorHealthy)
        {
            enterSafeMode(
                SafeModeReason::SensorFail);

            return;
        }

        if (!m_runtime.storageHealthy)
        {
            enterSafeMode(
                SafeModeReason::StorageFail);

            return;
        }
    }

    void RecoveryManager::enterSafeMode(
        SafeModeReason reason)
    {
        m_runtime.safeMode = true;

        m_runtime.heaterOn = false;

        m_runtime.humidifierOn = false;

        m_runtime.turnerOn = false;

        m_runtime.fanPwm = 0;

        m_recovery.safeModeReason = reason;
    }
}
```

---

# 7. SafeMode 정책

## 핵심 원칙

```text
출력 차단 우선
```

---

## 차단 대상

```text
Heater
Humidifier
Turner
Fan
```

---

# 8. Boot Recovery 흐름

```text
NVS Init
    ↓
Load Settings
    ↓
Load Batch
    ↓
Load Plan
    ↓
Validation
    ↓
SafeMode 판단
```

---

# 9. WDT Recovery 전략

## 정책

```text
반복 WDT Reset
    ↓
SafeMode Boot
```

---

## 권장 기준

```text
10분 내 3회 이상
```

---

# 10. Factory Reset 준비

## 삭제 대상

```text
Settings
Batch
Plan
WiFi Credentials
Recovery Flags
```

---

# 11. Main Loop 연결

## main.cpp

```cpp
void setup()
{
    recovery.boot();
}

void loop()
{
    const uint32_t now = millis();

    sensorManager.tick(now);

    scheduler.tick(now);

    climate.tick(now);

    alarm.tick(now);

    recovery.tick(now);

    appController.tick();

    ui.tick(now);

    cloud.tick(now);
}
```

---

# 12. 핵심 장점

## 1) SafeMode 중앙화

```text
SafeMode 진입 조건을 한 곳에서 관리
```

---

## 2) 출력 차단 일관성

모든 위험 출력 OFF.

---

## 3) Boot Recovery 단순화

```text
Load
 ↓
Validate
 ↓
Recover
```

구조 유지.

---

# 13. 금지 사항

```text
❌ Recovery 내부 UI Draw

❌ Recovery 내부 MQTT Publish

❌ Recovery 내부 delay()

❌ Recovery 내부 blocking wait
```

---

# 14. Acceptance Criteria

```text
AC-1
sensorHealthy=false 시 SafeMode

AC-2
SafeMode 시 출력 OFF

AC-3
BootCount 증가

AC-4
storageHealthy=false 시 SafeMode

AC-5
Main Loop blocking 없음
```

---

# 15. 다음 단계

다음 DDU:

```text
DDU-STORAGE-001
NVS Settings Storage
```

다음 구현 예정:

- NVS Save
- Settings Validation
- Atomic Save
- Persistent Header
- Schema Version
