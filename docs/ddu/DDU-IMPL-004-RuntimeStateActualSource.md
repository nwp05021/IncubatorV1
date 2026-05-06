
# DDU-IMPL-004 — RuntimeState Actual Source

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + PlatformIO

---

# 1. 목적

실제 구현 가능한 RuntimeState 기본 소스를 정의한다.

목표:

- Single Source of Truth
- Plain Struct 유지
- 동적 메모리 제거
- UI/Cloud 공용 상태 제공

---

# 2. RuntimeState.h

```cpp
#pragma once

#include <stdint.h>

namespace incubator::domain
{
    enum class SystemMode
    {
        Boot,
        Auto,
        Manual,
        SafeMode
    };

    struct RuntimeState
    {
        // ---------- System ----------

        SystemMode mode =
            SystemMode::Boot;

        bool safeMode = false;

        bool sensorHealthy = false;

        bool storageHealthy = false;

        // ---------- Temperature ----------

        float currentTempC = 0.0f;

        float targetTempC = 37.5f;

        // ---------- Humidity ----------

        float currentHumidityPct = 0.0f;

        float targetHumidityPct = 60.0f;

        // ---------- Output ----------

        bool heaterOn = false;

        bool humidifierOn = false;

        bool turnerOn = false;

        uint8_t fanPwm = 0;

        // ---------- Schedule ----------

        uint16_t currentDay = 0;

        uint16_t totalDays = 21;

        bool lockdown = false;

        // ---------- Alarm ----------

        bool highTempAlarm = false;

        bool lowTempAlarm = false;

        // ---------- Connectivity ----------

        bool wifiConnected = false;

        bool awsConnected = false;

        // ---------- Runtime ----------

        uint32_t uptimeMs = 0;
    };
}
```

---

# 3. 핵심 원칙

```text
RuntimeState는
계산 로직을 가지지 않는다.
```

---

# 4. 금지 사항

```text
❌ method 추가

❌ business logic 추가

❌ dynamic memory 사용

❌ device pointer 보관
```

---

# 5. 핵심 장점

## Plain Struct

디버깅 단순화.

---

## Shared State

UI/Cloud/Event 공유 가능.

---

## Predictable Memory

Heap 안정성 확보.

---

# 6. Acceptance Criteria

```text
AC-1
Plain Struct 유지

AC-2
동적 메모리 없음

AC-3
초기값 안전

AC-4
출력 초기값 OFF

AC-5
SafeMode 초기값 false
```
