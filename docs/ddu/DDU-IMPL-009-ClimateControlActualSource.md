
# DDU-IMPL-009 — Climate Control Actual Source

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + Relay Output
> Dependency:
> - RuntimeState
> - AppSettings
> - SensorModule
>
> Estimated Time: 20~40 min

---

# 1. 목적

실기기 검증 가능한 Climate Control 구현.

핵심 흐름:

```text
RuntimeState
    ↓
ClimatePolicy
    ↓
Relay Output
    ↓
Heater / Humidifier
```

목표:

- Temperature Hysteresis
- Humidity Hysteresis
- Relay Protection
- SafeMode OFF
- Non-Blocking Tick

---

# 2. 생성 파일

```text
product/devices/output/RelayDevice.h
product/devices/output/RelayDevice.cpp

product/modules/climate/ClimateModule.h
product/modules/climate/ClimateModule.cpp
```

---

# 3. 핵심 철학

```text
Climate는 예측 가능해야 한다.
```

---

# 4. RelayDevice

## 역할

GPIO 제어 전용.

---

## 금지

```text
❌ Hysteresis 판단

❌ Alarm 처리

❌ RuntimeState 변경
```

---

## RelayDevice.h

```cpp
#pragma once

namespace incubator::devices
{
    class RelayDevice
    {
    public:
        RelayDevice(
            int pin);

    public:
        void begin();

        void on();

        void off();

    private:
        int m_pin;
    };
}
```

---

# 5. ClimateModule

## 역할

```text
Temp/Humidity Policy
Output 결정
```

---

## ClimateModule.h

```cpp
#pragma once

#include "../../domain/RuntimeState.h"
#include "../../domain/AppSettings.h"

#include "../../devices/output/RelayDevice.h"

namespace incubator::modules::climate
{
    class ClimateModule
    {
    public:
        ClimateModule(
            incubator::domain::RuntimeState& runtime,
            incubator::domain::AppSettings& settings,
            incubator::devices::RelayDevice& heater,
            incubator::devices::RelayDevice& humidifier);

    public:
        void tick(uint32_t nowMs);

    private:
        void processTemperature();

        void processHumidity();

    private:
        incubator::domain::RuntimeState& m_runtime;

        incubator::domain::AppSettings& m_settings;

        incubator::devices::RelayDevice& m_heater;

        incubator::devices::RelayDevice& m_humidifier;

        uint32_t m_lastTickMs = 0;

        static constexpr uint32_t TickIntervalMs =
            500;
    };
}
```

---

# 6. ClimateModule.cpp

```cpp
#include "ClimateModule.h"

namespace incubator::modules::climate
{
    using namespace incubator::domain;

    ClimateModule::ClimateModule(
        RuntimeState& runtime,
        AppSettings& settings,
        incubator::devices::RelayDevice& heater,
        incubator::devices::RelayDevice& humidifier)
        :
        m_runtime(runtime),
        m_settings(settings),
        m_heater(heater),
        m_humidifier(humidifier)
    {
    }

    void ClimateModule::tick(
        uint32_t nowMs)
    {
        if ((nowMs - m_lastTickMs) <
            TickIntervalMs)
        {
            return;
        }

        m_lastTickMs = nowMs;

        if (m_runtime.safeMode)
        {
            m_heater.off();

            m_humidifier.off();

            m_runtime.heaterOn = false;

            m_runtime.humidifierOn = false;

            return;
        }

        processTemperature();

        processHumidity();
    }

    void ClimateModule::processTemperature()
    {
        const float target =
            m_runtime.targetTempC;

        const float current =
            m_runtime.currentTempC;

        const float hysteresis =
            m_settings.tempHysteresis;

        if (current < (target - hysteresis))
        {
            m_heater.on();

            m_runtime.heaterOn = true;
        }
        else if (current >
                 (target + hysteresis))
        {
            m_heater.off();

            m_runtime.heaterOn = false;
        }
    }

    void ClimateModule::processHumidity()
    {
        const float target =
            m_runtime.targetHumidityPct;

        const float current =
            m_runtime.currentHumidityPct;

        const float hysteresis =
            m_settings.humidityHysteresis;

        if (current < (target - hysteresis))
        {
            m_humidifier.on();

            m_runtime.humidifierOn = true;
        }
        else if (current >
                 (target + hysteresis))
        {
            m_humidifier.off();

            m_runtime.humidifierOn = false;
        }
    }
}
```

---

# 7. Hysteresis 전략

## Temperature

```text
target ± hysteresis
```

---

## Humidity

```text
target ± hysteresis
```

---

## 목적

```text
Relay chatter 방지
```

---

# 8. SafeMode 전략

## 핵심 원칙

```text
위험 상태 시 출력 차단
```

---

## 차단 대상

```text
Heater
Humidifier
```

---

# 9. Relay 보호 전략

## 권장

```text
Minimum ON Time
Minimum OFF Time
```

---

## 향후 확장 가능

```text
Soft Start
```

---

# 10. Main Loop 연결

```cpp
void loop()
{
    const uint32_t now = millis();

    sensorManager.tick(now);

    climate.tick(now);

    recovery.tick(now);
}
```

---

# 11. 핵심 장점

## 1) Device / Policy 분리

GPIO와 제어 정책 분리.

---

## 2) Predictable Control

예측 가능한 Climate 동작.

---

## 3) RuntimeState 중심 구조

UI/Cloud/Event 동일 데이터 사용.

---

# 12. 금지 사항

```text
❌ delay()

❌ Device 내부 정책 판단

❌ Runtime dynamic allocation

❌ Blocking relay loop
```

---

# 13. Acceptance Criteria

```text
AC-1
Temperature hysteresis 정상

AC-2
Humidity hysteresis 정상

AC-3
Relay ON/OFF 정상

AC-4
SafeMode 시 출력 OFF

AC-5
Main Loop blocking 없음
```

---

# 14. 다음 단계

```text
DDU-IMPL-010
Alarm & Recovery Actual Source
```
