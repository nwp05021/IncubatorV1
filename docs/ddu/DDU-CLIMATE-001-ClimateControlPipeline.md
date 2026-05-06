
# DDU-CLIMATE-001 — Climate Control Pipeline

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + PlatformIO
> Dependency:
> - RuntimeState
> - AppSettings
> - Sensor Pipeline
>
> Estimated Time: 20~40 min

---

# 1. 목적

환경 제어의 핵심 흐름을 구현한다.

```text
RuntimeState
    ↓
ClimateModule
    ↓
Output Decision
    ↓
Relay Device
```

목표:

- 히스테리시스 기반 제어
- Non-Blocking 구조
- SafeMode 보호
- RuntimeState 중심 제어

---

# 2. 생성 파일

```text
product/modules/climate/ClimateModule.h
product/modules/climate/ClimateModule.cpp

product/devices/relay/GpioRelayDevice.h
product/devices/relay/GpioRelayDevice.cpp
```

---

# 3. 핵심 철학

## ClimateModule 역할

ClimateModule은:

- 상태 계산
- 목표 비교
- 출력 결정

만 수행한다.

---

## 금지

```text
❌ delay()
❌ while polling
❌ UI 접근
❌ Cloud 접근
❌ 저장 처리
```

---

# 4. 상태 흐름

```text
Sensor Value
    ↓
RuntimeState
    ↓
ClimateModule
    ↓
Heater/Humidifier Decision
    ↓
Relay Device
```

---

# 5. 제어 전략

## Heater

```text
현재 온도 < 목표 - hysteresis
    → Heater ON

현재 온도 > 목표 + hysteresis
    → Heater OFF
```

---

## Humidifier

```text
현재 습도 < 목표 - hysteresis
    → Humidifier ON

현재 습도 > 목표 + hysteresis
    → Humidifier OFF
```

---

# 6. GpioRelayDevice

## 역할

GPIO 출력 전용.

---

## GpioRelayDevice.h

```cpp
#pragma once

#include <stdint.h>

namespace incubator::devices
{
    class GpioRelayDevice
    {
    public:
        explicit GpioRelayDevice(uint8_t pin);

    public:
        void begin();

        void set(bool on);

    private:
        uint8_t m_pin;
    };
}
```

---

# 7. ClimateModule

## ClimateModule.h

```cpp
#pragma once

#include "../../domain/RuntimeState.h"
#include "../../domain/AppSettings.h"

#include "../../devices/relay/GpioRelayDevice.h"

namespace incubator::modules::climate
{
    class ClimateModule
    {
    public:
        ClimateModule(
            incubator::domain::RuntimeState& runtime,
            incubator::domain::AppSettings& settings,
            incubator::devices::GpioRelayDevice& heater,
            incubator::devices::GpioRelayDevice& humidifier);

    public:
        void tick(uint32_t nowMs);

    private:
        void processHeater();
        void processHumidifier();

    private:
        incubator::domain::RuntimeState& m_runtime;

        incubator::domain::AppSettings& m_settings;

        incubator::devices::GpioRelayDevice& m_heater;

        incubator::devices::GpioRelayDevice& m_humidifier;

        uint32_t m_lastTickMs = 0;

        static constexpr uint32_t TickIntervalMs = 500;
    };
}
```

---

# 8. ClimateModule.cpp

```cpp
#include "ClimateModule.h"

namespace incubator::modules::climate
{
    using namespace incubator::domain;

    ClimateModule::ClimateModule(
        RuntimeState& runtime,
        AppSettings& settings,
        incubator::devices::GpioRelayDevice& heater,
        incubator::devices::GpioRelayDevice& humidifier)
        :
        m_runtime(runtime),
        m_settings(settings),
        m_heater(heater),
        m_humidifier(humidifier)
    {
    }

    void ClimateModule::tick(uint32_t nowMs)
    {
        if ((nowMs - m_lastTickMs) < TickIntervalMs)
        {
            return;
        }

        m_lastTickMs = nowMs;

        if (m_runtime.safeMode)
        {
            m_runtime.heaterOn = false;
            m_runtime.humidifierOn = false;

            m_heater.set(false);
            m_humidifier.set(false);

            return;
        }

        processHeater();

        processHumidifier();
    }

    void ClimateModule::processHeater()
    {
        const float current = m_runtime.currentTempC;
        const float target = m_runtime.targetTempC;

        const float hyst = m_settings.tempHysteresis;

        if (current < (target - hyst))
        {
            m_runtime.heaterOn = true;
        }
        else if (current > (target + hyst))
        {
            m_runtime.heaterOn = false;
        }

        m_heater.set(m_runtime.heaterOn);
    }

    void ClimateModule::processHumidifier()
    {
        const float current = m_runtime.currentHumidityPct;
        const float target = m_runtime.targetHumidityPct;

        const float hyst = m_settings.humidityHysteresis;

        if (current < (target - hyst))
        {
            m_runtime.humidifierOn = true;
        }
        else if (current > (target + hyst))
        {
            m_runtime.humidifierOn = false;
        }

        m_humidifier.set(m_runtime.humidifierOn);
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

    climate.tick(now);

    scheduler.tick(now);

    appController.tick();

    ui.tick(now);

    cloud.tick(now);
}
```

---

# 10. SafeMode 전략

## 핵심 원칙

```text
출력 차단 우선
```

---

## SafeMode 동작

```text
Heater OFF
Humidifier OFF
Turner OFF
Fan OFF
```

---

# 11. 핵심 장점

## 1) RuntimeState 중심 구조

```text
출력 상태도 RuntimeState에 기록된다.
```

---

## 2) 디버깅 단순화

```text
heaterOn
humidifierOn
```

현재 상태를 즉시 확인 가능.

---

## 3) UI / Cloud 동기화 단순화

```text
RuntimeState
    ↓
Telemetry
    ↓
UI
```

동일 데이터 사용.

---

# 12. 금지 사항

```text
❌ Climate 내부 delay()

❌ Relay Device 내부 정책

❌ Device 내부 상태 변경

❌ UI 직접 GPIO 접근
```

---

# 13. Acceptance Criteria

```text
AC-1
500ms Tick 기반 동작

AC-2
온도 hysteresis 정상 동작

AC-3
습도 hysteresis 정상 동작

AC-4
SafeMode 시 출력 OFF

AC-5
Main Loop blocking 없음
```

---

# 14. 다음 단계

다음 DDU:

```text
DDU-SCHED-001
Incubation Scheduler
```

다음 구현 예정:

- Batch Day 계산
- Lockdown 판단
- Plan Row 적용
- RuntimeState Target Update
- Day Change Event
