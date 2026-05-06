
# DDU-IMPL-008 — Sensor Pipeline Actual Source

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + AHT20
> Dependency:
> - RuntimeState
> - AlarmState
> - RecoveryManager
>
> Estimated Time: 20~40 min

---

# 1. 목적

실기기 검증 가능한 Sensor Pipeline 실제 구현.

핵심 흐름:

```text
Sensor Device
    ↓
SensorModule
    ↓
RuntimeState
    ↓
Alarm / Recovery
```

목표:

- Non-Blocking Poll
- Sensor Health
- RuntimeState Update
- Sensor Failure Detect
- SafeMode 연계

---

# 2. 생성 파일

```text
product/devices/sensor/Aht20Device.h
product/devices/sensor/Aht20Device.cpp

product/modules/sensor/SensorModule.h
product/modules/sensor/SensorModule.cpp
```

---

# 3. 핵심 철학

```text
센서는 데이터를 제공할 뿐
판단하지 않는다.
```

---

# 4. Aht20Device

## 역할

하드웨어 접근 전용.

---

## 금지

```text
❌ Alarm 판단

❌ RuntimeState 변경

❌ SafeMode 처리
```

---

## Aht20Device.h

```cpp
#pragma once

namespace incubator::devices
{
    class Aht20Device
    {
    public:
        bool begin();

        bool read(
            float& tempC,
            float& humidityPct);
    };
}
```

---

# 5. SensorModule

## 역할

```text
Sensor Poll
Health Check
RuntimeState Update
```

---

## SensorModule.h

```cpp
#pragma once

#include "../../domain/RuntimeState.h"

#include "../../devices/sensor/Aht20Device.h"

namespace incubator::modules::sensor
{
    class SensorModule
    {
    public:
        SensorModule(
            incubator::domain::RuntimeState& runtime,
            incubator::devices::Aht20Device& sensor);

    public:
        void tick(uint32_t nowMs);

    private:
        void processSensor();

    private:
        incubator::domain::RuntimeState& m_runtime;

        incubator::devices::Aht20Device& m_sensor;

        uint32_t m_lastPollMs = 0;

        uint32_t m_failCount = 0;

        static constexpr uint32_t PollIntervalMs =
            2000;

        static constexpr uint32_t MaxFailCount =
            3;
    };
}
```

---

# 6. SensorModule.cpp

```cpp
#include "SensorModule.h"

namespace incubator::modules::sensor
{
    using namespace incubator::domain;

    SensorModule::SensorModule(
        RuntimeState& runtime,
        incubator::devices::Aht20Device& sensor)
        :
        m_runtime(runtime),
        m_sensor(sensor)
    {
    }

    void SensorModule::tick(
        uint32_t nowMs)
    {
        if ((nowMs - m_lastPollMs) <
            PollIntervalMs)
        {
            return;
        }

        m_lastPollMs = nowMs;

        processSensor();
    }

    void SensorModule::processSensor()
    {
        float tempC = 0.0f;

        float humidityPct = 0.0f;

        if (!m_sensor.read(
            tempC,
            humidityPct))
        {
            ++m_failCount;

            if (m_failCount >= MaxFailCount)
            {
                m_runtime.sensorHealthy =
                    false;
            }

            return;
        }

        m_failCount = 0;

        m_runtime.sensorHealthy = true;

        m_runtime.currentTempC =
            tempC;

        m_runtime.currentHumidityPct =
            humidityPct;
    }
}
```

---

# 7. Sensor Failure 전략

## 핵심 원칙

```text
센서 오류는 감춰지면 안 된다.
```

---

## 조건

```text
연속 3회 read fail
```

---

## 결과

```text
sensorHealthy = false
```

---

# 8. Recovery 연계

## 핵심 흐름

```text
sensorHealthy == false
    ↓
RecoveryManager
    ↓
SafeMode
```

---

# 9. Poll 정책

| 항목 | 값 |
|---|---|
| Poll Interval | 2 sec |
| Fail Count | 3 |
| Blocking Delay | 금지 |

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

## 1) Sensor / Policy 분리

Device와 Module 역할 명확.

---

## 2) Health 기반 구조

Recovery 연계 가능.

---

## 3) RuntimeState 중심 구조

UI/Cloud/Event 공용 사용 가능.

---

# 12. 금지 사항

```text
❌ Device 내부 정책 판단

❌ Sensor direct SafeMode

❌ delay()

❌ Runtime dynamic allocation
```

---

# 13. Acceptance Criteria

```text
AC-1
AHT20 read 정상

AC-2
RuntimeState update 정상

AC-3
3회 fail 감지

AC-4
sensorHealthy 갱신

AC-5
Main Loop blocking 없음
```

---

# 14. 다음 단계

```text
DDU-IMPL-009
Climate Control Actual Source
```
