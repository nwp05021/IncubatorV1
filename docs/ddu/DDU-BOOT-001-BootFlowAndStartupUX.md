
# DDU-BOOT-001 — Boot Flow + Startup UX

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + ST7789
> Dependency:
> - RecoveryManager
> - SettingsStorage
> - RuntimeState
>
> Estimated Time: 20~40 min

---

# 1. 목적

안전한 부팅 및 Premium Startup UX 구현.

핵심 흐름:

```text
Power On
    ↓
Storage Restore
    ↓
Device Init
    ↓
Validation
    ↓
UI Boot Screen
    ↓
Loop Start
```

목표:

- Safe Boot
- Startup Validation
- Recovery UX
- Boot Progress
- Device Init Flow

---

# 2. 생성 파일

```text
product/boot/BootManager.h
product/boot/BootManager.cpp

product/ui/boot/BootScreenRenderer.h
product/ui/boot/BootScreenRenderer.cpp
```

---

# 3. 핵심 철학

```text
부팅 과정은
제품의 신뢰감을 결정한다.
```

---

# 4. Boot 단계

| 단계 | 목적 |
|---|---|
| NVS Init | 저장소 초기화 |
| Restore Settings | 설정 복원 |
| Restore Batch | Batch 복원 |
| Device Init | 센서/출력 초기화 |
| Validation | 무결성 검사 |
| Recovery Check | SafeMode 판단 |
| UI Ready | 화면 시작 |
| Loop Start | Main Loop 시작 |

---

# 5. BootManager

## 역할

```text
부팅 순서 관리
Validation
Recovery 연계
```

---

## BootManager.h

```cpp
#pragma once

#include "../domain/RuntimeState.h"
#include "../domain/RecoveryState.h"

namespace incubator::boot
{
    class BootManager
    {
    public:
        BootManager(
            incubator::domain::RuntimeState& runtime,
            incubator::domain::RecoveryState& recovery);

    public:
        bool boot();

    private:
        bool initializeStorage();

        bool restoreSettings();

        bool initializeDevices();

        bool validateSystem();

        void enterSafeBoot();

    private:
        incubator::domain::RuntimeState& m_runtime;

        incubator::domain::RecoveryState& m_recovery;
    };
}
```

---

# 6. BootManager.cpp

```cpp
#include "BootManager.h"

namespace incubator::boot
{
    using namespace incubator::domain;

    BootManager::BootManager(
        RuntimeState& runtime,
        RecoveryState& recovery)
        :
        m_runtime(runtime),
        m_recovery(recovery)
    {
    }

    bool BootManager::boot()
    {
        if (!initializeStorage())
        {
            enterSafeBoot();

            return false;
        }

        if (!restoreSettings())
        {
            enterSafeBoot();

            return false;
        }

        if (!initializeDevices())
        {
            enterSafeBoot();

            return false;
        }

        if (!validateSystem())
        {
            enterSafeBoot();

            return false;
        }

        return true;
    }

    bool BootManager::initializeStorage()
    {
        return true;
    }

    bool BootManager::restoreSettings()
    {
        return true;
    }

    bool BootManager::initializeDevices()
    {
        return true;
    }

    bool BootManager::validateSystem()
    {
        return true;
    }

    void BootManager::enterSafeBoot()
    {
        m_runtime.safeMode = true;

        m_recovery.safeModeReason =
            SafeModeReason::StorageFail;
    }
}
```

---

# 7. Startup Validation

## 검사 항목

| 항목 | 목적 |
|---|---|
| Settings | 범위 확인 |
| Plan | Row 확인 |
| Sensor | 응답 확인 |
| Display | 초기화 확인 |
| Storage | NVS 상태 확인 |

---

# 8. BootScreenRenderer

## 역할

```text
Startup Progress 표시
```

---

## 표시 예시

```text
INCUBATOR SYSTEM

INITIALIZING...
[██████------]

CHECKING SENSOR...
```

---

## BootScreenRenderer.h

```cpp
#pragma once

namespace incubator::ui
{
    class BootScreenRenderer
    {
    public:
        void showProgress(
            int percent,
            const char* message);
    };
}
```

---

# 9. Recovery UX

## Safe Boot

```text
SAFE MODE

CHECK STORAGE
```

---

## 핵심 목표

```text
문제가 무엇인지 즉시 이해
```

---

# 10. Safe Boot 전략

## 핵심 원칙

```text
불확실하면 출력 금지
```

---

## 차단 대상

```text
Heater
Humidifier
Fan
Stepper
```

---

# 11. Main Boot Flow

```text
setup()
    ↓
BootManager.boot()
    ↓
Recovery 판단
    ↓
UI Boot Screen
    ↓
loop()
```

---

# 12. Main.cpp 연결

## setup()

```cpp
void setup()
{
    Serial.begin(115200);

    boot.boot();

    recovery.boot();
}
```

---

# 13. Premium Startup UX

## 목표

```text
완성된 산업 장비 느낌
```

---

## 권장

```text
Soft Progress
Simple Layout
Fast Response
```

---

## 금지

```text
❌ 긴 Splash

❌ 화려한 Animation

❌ Blocking Delay
```

---

# 14. 핵심 장점

## 1) Safe Boot 구조

초기 위험 상태 차단 가능.

---

## 2) Startup Validation 중앙화

문제 진단 단순화.

---

## 3) Premium UX

신뢰감 향상.

---

# 15. 금지 사항

```text
❌ setup() 내부 delay()

❌ Blocking Boot Animation

❌ GPIO direct output before validation

❌ Recovery bypass
```

---

# 16. Acceptance Criteria

```text
AC-1
Boot Flow 정상 동작

AC-2
Storage Validation 정상

AC-3
Device Init 정상

AC-4
Safe Boot 진입 가능

AC-5
Boot Progress 표시
```

---

# 17. 다음 단계

다음 DDU:

```text
DDU-DIAG-001
Diagnostics + Service Screen
```

다음 구현 예정:

- Sensor Diagnostics
- GPIO Diagnostics
- WiFi Diagnostics
- Factory Service Mode
- Debug Overlay
