
# DDU-SENSOR-001 — Sensor Pipeline

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + PlatformIO
> Dependency:
> - RuntimeState
> - AppController
> - AHT20 Device
>
> Estimated Time: 20~40 min

---

# 1. 목적

센서 시스템의 핵심 데이터 흐름을 구현한다.

목표:

```text
AHT20
    ↓
SensorManager
    ↓
RuntimeState
    ↓
Alarm / Climate / UI
```

핵심 목표:

```text
센서가 시스템 전체 데이터의 단일 원천이 된다.
```

---

# 2. 생성 파일

```text
product/modules/sensor/SensorSample.h
product/modules/sensor/SensorManager.h
product/modules/sensor/SensorManager.cpp

product/devices/aht20/Aht20Device.h
product/devices/aht20/Aht20Device.cpp
```

---

# 3. 핵심 철학

## SensorManager 역할

SensorManager는:

- 비동기 센서 읽기
- RuntimeState 갱신
- 센서 상태 감시

만 수행한다.

금지:

```text
❌ Heater 제어
❌ Alarm Overlay
❌ GPIO 정책 판단
❌ Cloud Publish
```

---

# 4. 상태 흐름

```text
AHT20
    ↓
SensorSample
    ↓
SensorManager
    ↓
RuntimeState Update
    ↓
Sensor Health Check
```

---

# 5. SensorSample

## SensorSample.h

```cpp
#pragma once

#include <stdint.h>

namespace incubator::modules::sensor
{
    struct SensorSample
    {
        float temperatureC = 0.0f;
        float humidityPct = 0.0f;

        bool valid = false;

        uint32_t timestampMs = 0;
    };
}
```

---

# 6. Aht20Device

## 역할

하드웨어 접근 전용.

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
            float& temperatureC,
            float& humidityPct);
    };
}
```

---

# 7. SensorManager

## 역할

```text
센서 Polling
RuntimeState 갱신
센서 상태 확인
```

---

## SensorManager.h

```cpp
#pragma once

#include "../../domain/RuntimeState.h"
#include "SensorSample.h"
#include "../../devices/aht20/Aht20Device.h"

namespace incubator::modules::sensor
{
    class SensorManager
    {
    public:
        SensorManager(
            incubator::domain::RuntimeState& runtime,
            incubator::devices::Aht20Device& device);

    public:
        void tick(uint32_t nowMs);

    private:
        bool pollSensor(uint32_t nowMs);

    private:
        incubator::domain::RuntimeState& m_runtime;

        incubator::devices::Aht20Device& m_device;

        uint32_t m_lastPollMs = 0;

        static constexpr uint32_t PollIntervalMs = 2000;

        static constexpr uint32_t SensorTimeoutMs = 10000;
    };
}
```

---

# 8. SensorManager.cpp

```cpp
#include "SensorManager.h"

namespace incubator::modules::sensor
{
    using namespace incubator::domain;

    SensorManager::SensorManager(
        RuntimeState& runtime,
        incubator::devices::Aht20Device& device)
        :
        m_runtime(runtime),
        m_device(device)
    {
    }

    void SensorManager::tick(uint32_t nowMs)
    {
        if ((nowMs - m_lastPollMs) >= PollIntervalMs)
        {
            m_lastPollMs = nowMs;

            pollSensor(nowMs);
        }

        if ((nowMs - m_runtime.lastSensorUpdateMs) >= SensorTimeoutMs)
        {
            m_runtime.sensorHealthy = false;
            m_runtime.sensorFailAlarm = true;
        }
    }

    bool SensorManager::pollSensor(uint32_t nowMs)
    {
        float temp = 0.0f;
        float humi = 0.0f;

        if (!m_device.read(temp, humi))
        {
            m_runtime.sensorHealthy = false;

            return false;
        }

        m_runtime.currentTempC = temp;
        m_runtime.currentHumidityPct = humi;

        m_runtime.lastSensorUpdateMs = nowMs;

        m_runtime.sensorHealthy = true;

        return true;
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

# 10. 핵심 장점

## 1) RuntimeState 중심 구조

```text
센서 데이터는 반드시 RuntimeState를 통과한다.
```

---

## 2) Cloud/UI 독립성

```text
UI는 Sensor 직접 접근 금지
Cloud는 Sensor 직접 접근 금지
```

---

## 3) SafeMode 확장 용이

```text
sensorHealthy == false
    ↓
Recovery Command
    ↓
SafeMode
```

---

# 11. 금지 사항

```text
❌ delay()
❌ blocking read loop
❌ Sensor 내부 Alarm 처리
❌ Sensor 내부 GPIO 제어
❌ RuntimeState direct mutation from UI
```

---

# 12. Acceptance Criteria

```text
AC-1
2초 주기 Polling 정상 동작

AC-2
RuntimeState currentTempC 갱신

AC-3
센서 제거 시 sensorHealthy=false

AC-4
10초 timeout 시 sensorFailAlarm=true

AC-5
Main Loop blocking 없음
```

---

# 13. 다음 단계

다음 DDU:

```text
DDU-CLIMATE-001
Climate Control Pipeline
```

다음 구현 예정:

- Hysteresis
- Heater Control
- Humidifier Control
- Fan Policy
- Lockdown Fan Logic
- SafeMode Output Block
