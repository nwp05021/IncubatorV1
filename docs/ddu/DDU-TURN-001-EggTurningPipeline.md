
# DDU-TURN-001 — Egg Turning Pipeline

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + Stepper Driver
> Dependency:
> - Scheduler
> - RuntimeState
> - AppSettings
>
> Estimated Time: 20~40 min

---

# 1. 목적

자동 전란(Turning) 제어 구조 구현.

핵심 흐름:

```text
Scheduler
    ↓
TurningPolicy
    ↓
TurnCommand
    ↓
Stepper Device
```

목표:

- Turning Scheduler
- Turning Interval
- Lockdown Stop
- Stepper Recovery
- Safe Turning

---

# 2. 생성 파일

```text
product/modules/turning/TurningModule.h
product/modules/turning/TurningModule.cpp

product/devices/stepper/StepperDevice.h
product/devices/stepper/StepperDevice.cpp
```

---

# 3. 핵심 철학

```text
전란은 천천히
안전하게
예측 가능하게
```

---

# 4. StepperDevice

## 역할

하드웨어 제어 전용.

---

## 금지

```text
❌ Turning 정책 판단

❌ RuntimeState 변경
```

---

## StepperDevice.h

```cpp
#pragma once

namespace incubator::devices
{
    class StepperDevice
    {
    public:
        void begin();

        void rotateForward();

        void rotateBackward();

        void stop();
    };
}
```

---

# 5. TurningModule

## 역할

```text
Turning Schedule
Turning Timing
Lockdown Block
```

---

## TurningModule.h

```cpp
#pragma once

#include "../../domain/RuntimeState.h"
#include "../../domain/AppSettings.h"

#include "../../devices/stepper/StepperDevice.h"

namespace incubator::modules::turning
{
    class TurningModule
    {
    public:
        TurningModule(
            incubator::domain::RuntimeState& runtime,
            incubator::domain::AppSettings& settings,
            incubator::devices::StepperDevice& stepper);

    public:
        void tick(uint32_t nowMs);

    private:
        void processTurning(uint32_t nowMs);

    private:
        incubator::domain::RuntimeState& m_runtime;

        incubator::domain::AppSettings& m_settings;

        incubator::devices::StepperDevice& m_stepper;

        uint32_t m_lastTurnMs = 0;

        bool m_forward = true;

        static constexpr uint32_t TickIntervalMs =
            1000;
    };
}
```

---

# 6. TurningModule.cpp

```cpp
#include "TurningModule.h"

namespace incubator::modules::turning
{
    using namespace incubator::domain;

    TurningModule::TurningModule(
        RuntimeState& runtime,
        AppSettings& settings,
        incubator::devices::StepperDevice& stepper)
        :
        m_runtime(runtime),
        m_settings(settings),
        m_stepper(stepper)
    {
    }

    void TurningModule::tick(uint32_t nowMs)
    {
        if (m_runtime.safeMode)
        {
            m_stepper.stop();

            m_runtime.turnerOn = false;

            return;
        }

        processTurning(nowMs);
    }

    void TurningModule::processTurning(
        uint32_t nowMs)
    {
        if (m_runtime.lockdown)
        {
            m_stepper.stop();

            m_runtime.turnerOn = false;

            return;
        }

        const uint32_t interval =
            m_settings.turningIntervalMs;

        if ((nowMs - m_lastTurnMs) < interval)
        {
            return;
        }

        m_lastTurnMs = nowMs;

        if (m_forward)
        {
            m_stepper.rotateForward();
        }
        else
        {
            m_stepper.rotateBackward();
        }

        m_forward = !m_forward;

        m_runtime.turnerOn = true;
    }
}
```

---

# 7. Turning 정책

## 기본 정책

| 항목 | 값 |
|---|---|
| Interval | 2h |
| Duration | 10 sec |
| Lockdown Stop | Enabled |

---

# 8. Lockdown 전략

## 핵심 원칙

```text
Lockdown 시 전란 중단
```

---

## 조건

```text
runtime.lockdown == true
```

---

# 9. Turning Recovery 전략

## 목적

Stepper 오작동 보호.

---

## 권장

```text
Turning Timeout
```

---

## 예시

```text
15 sec 초과
    ↓
Stop
    ↓
Alarm
```

---

# 10. SafeMode 전략

## 핵심 원칙

```text
위험 출력 즉시 정지
```

---

## 차단 대상

```text
Stepper
```

---

# 11. Main Loop 연결

## main.cpp

```cpp
void loop()
{
    const uint32_t now = millis();

    sensorManager.tick(now);

    scheduler.tick(now);

    climate.tick(now);

    turning.tick(now);

    alarm.tick(now);

    recovery.tick(now);

    appController.tick();

    ui.tick(now);

    wifi.tick(now);

    aws.tick(now);

    shadow.tick(now);
}
```

---

# 12. RuntimeState 연계

## 기록 항목

```text
turnerOn
lockdown
```

---

## 금지

```text
❌ Stepper 내부 상태 변경
```

---

# 13. 핵심 장점

## 1) Scheduler 분리

Turning 정책 독립성 확보.

---

## 2) Lockdown 보호 단순화

RuntimeState 기반 차단.

---

## 3) Safe Turning 구조

예측 가능한 동작 유지.

---

# 14. 금지 사항

```text
❌ delay()

❌ Stepper 내부 정책 판단

❌ RuntimeState direct mutation from Device

❌ Blocking Stepper loop
```

---

# 15. Acceptance Criteria

```text
AC-1
Turning Interval 정상 동작

AC-2
Forward/Backward 정상 전환

AC-3
Lockdown 시 전란 중단

AC-4
SafeMode 시 Stepper 정지

AC-5
Main Loop blocking 없음
```

---

# 16. 다음 단계

다음 DDU:

```text
DDU-FAN-001
Air Circulation Pipeline
```

다음 구현 예정:

- PWM Fan Control
- Temp Adaptive Fan
- Lockdown Fan Policy
- Ambient Air UX
