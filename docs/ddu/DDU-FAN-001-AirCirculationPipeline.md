
# DDU-FAN-001 — Air Circulation Pipeline

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 PWM Fan
> Dependency:
> - RuntimeState
> - AppSettings
> - ClimateModule
>
> Estimated Time: 20~40 min

---

# 1. 목적

공기 순환(Fan) 제어 구조 구현.

핵심 흐름:

```text
RuntimeState
    ↓
FanPolicy
    ↓
PWM Output
    ↓
Fan Device
```

목표:

- PWM Fan Control
- Temp Adaptive Fan
- Lockdown Fan Policy
- Quiet Ambient UX
- SafeMode Protection

---

# 2. 생성 파일

```text
product/modules/fan/FanModule.h
product/modules/fan/FanModule.cpp

product/devices/fan/PwmFanDevice.h
product/devices/fan/PwmFanDevice.cpp
```

---

# 3. 핵심 철학

```text
Fan은 보조 장치가 아니라
환경 안정화 장치다.
```

---

# 4. PwmFanDevice

## 역할

PWM 출력 전용.

---

## 금지

```text
❌ Fan 정책 판단

❌ RuntimeState 변경
```

---

## PwmFanDevice.h

```cpp
#pragma once

#include <stdint.h>

namespace incubator::devices
{
    class PwmFanDevice
    {
    public:
        void begin();

        void setPwm(
            uint8_t duty);
    };
}
```

---

# 5. FanModule

## 역할

```text
Fan Policy
PWM 결정
Adaptive Control
```

---

## FanModule.h

```cpp
#pragma once

#include "../../domain/RuntimeState.h"
#include "../../domain/AppSettings.h"

#include "../../devices/fan/PwmFanDevice.h"

namespace incubator::modules::fan
{
    class FanModule
    {
    public:
        FanModule(
            incubator::domain::RuntimeState& runtime,
            incubator::domain::AppSettings& settings,
            incubator::devices::PwmFanDevice& fan);

    public:
        void tick(uint32_t nowMs);

    private:
        void processFan();

    private:
        incubator::domain::RuntimeState& m_runtime;

        incubator::domain::AppSettings& m_settings;

        incubator::devices::PwmFanDevice& m_fan;

        uint32_t m_lastTickMs = 0;

        static constexpr uint32_t TickIntervalMs =
            1000;
    };
}
```

---

# 6. FanModule.cpp

```cpp
#include "FanModule.h"

namespace incubator::modules::fan
{
    using namespace incubator::domain;

    FanModule::FanModule(
        RuntimeState& runtime,
        AppSettings& settings,
        incubator::devices::PwmFanDevice& fan)
        :
        m_runtime(runtime),
        m_settings(settings),
        m_fan(fan)
    {
    }

    void FanModule::tick(uint32_t nowMs)
    {
        if ((nowMs - m_lastTickMs) < TickIntervalMs)
        {
            return;
        }

        m_lastTickMs = nowMs;

        processFan();
    }

    void FanModule::processFan()
    {
        if (m_runtime.safeMode)
        {
            m_runtime.fanPwm = 0;

            m_fan.setPwm(0);

            return;
        }

        uint8_t pwm =
            m_settings.fanNormalPwm;

        if (m_runtime.lockdown)
        {
            pwm =
                m_settings.fanLockdownPwm;
        }

        if (m_runtime.currentTempC >
            m_runtime.targetTempC + 0.5f)
        {
            pwm += 10;
        }

        if (pwm > 100)
        {
            pwm = 100;
        }

        m_runtime.fanPwm = pwm;

        m_fan.setPwm(pwm);
    }
}
```

---

# 7. Fan 정책

## 기본 정책

| 상태 | PWM |
|---|---|
| Idle | 20 |
| Normal | 40 |
| Lockdown | 60 |
| High Temp | +10 |

---

# 8. Temp Adaptive 전략

## 정책

```text
현재 온도 > 목표 + 0.5°C
    ↓
Fan Boost
```

---

## 목적

```text
온도 균일화
```

---

# 9. Lockdown Fan 전략

## 목적

```text
부화 직전 산소 공급 강화
```

---

## 정책

```text
Lockdown 시 PWM 증가
```

---

# 10. Quiet Ambient UX

## 핵심 목표

```text
소음 최소화
```

---

## 권장

```text
급격한 PWM 변화 금지
```

---

## 향후 확장

```text
Soft Ramp
```

권장.

---

# 11. SafeMode 전략

## 핵심 원칙

```text
위험 상태 시 출력 차단
```

---

## SafeMode

```text
Fan OFF
```

---

# 12. RuntimeState 연계

## 기록 항목

```text
fanPwm
```

---

## 금지

```text
❌ Device 내부 상태 변경
```

---

# 13. Main Loop 연결

## main.cpp

```cpp
void loop()
{
    const uint32_t now = millis();

    sensorManager.tick(now);

    scheduler.tick(now);

    climate.tick(now);

    turning.tick(now);

    fan.tick(now);

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

# 14. 핵심 장점

## 1) Adaptive Fan 구조

환경 변화 대응 가능.

---

## 2) RuntimeState 중심 구조

Cloud/UI 동일 데이터 사용.

---

## 3) Quiet UX

상용 장비 느낌 강화.

---

# 15. 금지 사항

```text
❌ delay()

❌ Fan Device 내부 정책 판단

❌ Blocking PWM loop

❌ Runtime dynamic allocation
```

---

# 16. Acceptance Criteria

```text
AC-1
PWM 정상 출력

AC-2
Temp Adaptive 동작

AC-3
Lockdown PWM 증가

AC-4
SafeMode 시 Fan OFF

AC-5
Main Loop blocking 없음
```

---

# 17. 다음 단계

다음 DDU:

```text
DDU-BOOT-001
Boot Flow + Startup UX
```

다음 구현 예정:

- Boot Logo
- Startup Validation
- Recovery Screen
- Device Init Flow
- Safe Boot
