# Runtime Core v1 — RuntimeState / AppSettings / CommandQueue / AppController

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + PlatformIO
> Architecture: Command-Driven + RuntimeState-Centric

---

# 1. 목표

이 문서는 다음 핵심 Runtime 기반 구조를 실제 구현 가능한 수준으로 고정한다.

구현 대상:

* RuntimeState
* AppSettings
* Command
* CommandQueue
* AppController
* Validation
* Tick 기반 처리

핵심 목표:

```text
사람이 상태 흐름을 즉시 이해 가능한 구조
```

---

# 2. 프로젝트 구조

```text
product/
├── app/
│   ├── AppController.h
│   ├── AppController.cpp
│   ├── Command.h
│   ├── CommandQueue.h
│   └── CommandQueue.cpp
│
├── domain/
│   ├── RuntimeState.h
│   ├── AppSettings.h
│   ├── BatchState.h
│   └── PlanRow.h
│
├── modules/
├── ui/
├── cloud/
├── recovery/
└── devices/
```

---

# 3. RuntimeState

## 목적

현재 시스템 상태의 유일한 진실.

읽기:

* UI
* Cloud
* Telemetry
* Alarm
* Debug

쓰기:

* Modules
* Scheduler
* Recovery
* AppController

---

## RuntimeState.h

```cpp
#pragma once

#include <stdint.h>

namespace incubator::domain
{
    enum class SystemMode
    {
        Idle,
        Running,
        Manual,
        SafeMode
    };

    struct RuntimeState
    {
        // Sensor
        float currentTempC = 0.0f;
        float currentHumidityPct = 0.0f;

        // Target
        float targetTempC = 37.5f;
        float targetHumidityPct = 60.0f;

        // Output
        bool heaterOn = false;
        bool humidifierOn = false;
        bool turnerOn = false;
        uint8_t fanPwm = 0;

        // Batch
        bool batchActive = false;
        uint16_t currentDay = 0;
        uint16_t totalDays = 21;
        bool lockdown = false;

        // Network
        bool wifiConnected = false;
        bool awsConnected = false;

        // Alarm
        bool highTempAlarm = false;
        bool lowTempAlarm = false;
        bool sensorFailAlarm = false;

        // System
        bool safeMode = false;
        bool storageHealthy = true;
        bool sensorHealthy = true;

        SystemMode mode = SystemMode::Idle;

        uint32_t uptimeMs = 0;
        uint32_t lastSensorUpdateMs = 0;
    };
}
```

---

# 4. AppSettings

## 목적

정적 운영 설정.

특징:

```text
저장 위치: NVS
변경 빈도: 낮음
변경 주체: AppController
```

---

## AppSettings.h

```cpp
#pragma once

#include <stdint.h>

namespace incubator::domain
{
    struct AppSettings
    {
        float tempHysteresis = 0.3f;
        float humidityHysteresis = 3.0f;

        uint32_t telemetryIntervalMs = 60000;

        bool cloudEnabled = true;
        bool alarmEnabled = true;

        uint32_t turningIntervalMs = 7200000;
        uint16_t turningDurationSec = 10;

        uint8_t fanIdlePwm = 20;
        uint8_t fanNormalPwm = 40;
        uint8_t fanLockdownPwm = 60;
    };
}
```

---

# 5. Command 구조

## 핵심 철학

모든 외부 입력은 Command로 변환한다.

---

## Command.h

```cpp
#pragma once

#include <stdint.h>

namespace incubator::app
{
    enum class CommandType
    {
        None,

        StartBatch,
        StopBatch,

        EnterSafeMode,
        ClearSafeMode,

        SetTargetTemperature,
        SetTargetHumidity,

        ToggleManualHeater,
        ToggleManualHumidifier,

        PatchSettings,
    };

    enum class CommandSource
    {
        Ui,
        Cloud,
        Ble,
        Recovery,
        Scheduler,
        System
    };

    struct SetFloatPayload
    {
        float value;
    };

    struct TogglePayload
    {
        bool enabled;
    };

    struct Command
    {
        CommandType type = CommandType::None;

        CommandSource source = CommandSource::System;

        uint32_t timestampMs = 0;

        union
        {
            SetFloatPayload setFloat;
            TogglePayload toggle;
        };
    };
}
```

---

# 6. CommandQueue

## 핵심 철학

```text
Single Threaded State Mutation
```

모든 상태 변경 요청은 Queue에 들어간다.

---

## CommandQueue.h

```cpp
#pragma once

#include "Command.h"

namespace incubator::app
{
    class CommandQueue
    {
    public:
        static constexpr uint8_t Capacity = 32;

    public:
        bool enqueue(const Command& cmd);
        bool dequeue(Command& cmd);

        bool isEmpty() const;
        bool isFull() const;

        uint8_t count() const;

    private:
        Command m_buffer[Capacity];

        uint8_t m_head = 0;
        uint8_t m_tail = 0;
        uint8_t m_count = 0;
    };
}
```

---

## CommandQueue.cpp

```cpp
#include "CommandQueue.h"

namespace incubator::app
{
    bool CommandQueue::enqueue(const Command& cmd)
    {
        if (isFull())
        {
            return false;
        }

        m_buffer[m_tail] = cmd;

        m_tail = (m_tail + 1) % Capacity;

        ++m_count;

        return true;
    }

    bool CommandQueue::dequeue(Command& cmd)
    {
        if (isEmpty())
        {
            return false;
        }

        cmd = m_buffer[m_head];

        m_head = (m_head + 1) % Capacity;

        --m_count;

        return true;
    }

    bool CommandQueue::isEmpty() const
    {
        return m_count == 0;
    }

    bool CommandQueue::isFull() const
    {
        return m_count >= Capacity;
    }

    uint8_t CommandQueue::count() const
    {
        return m_count;
    }
}
```

---

# 7. AppController

## 목적

모든 상태 변경의 중앙 통제.

---

## 책임

| 기능           | 역할    |
| ------------ | ----- |
| Validation   | 위험 차단 |
| Mutation     | 상태 변경 |
| Persistence  | 저장 요청 |
| Event Notify | 상태 통지 |

---

## AppController.h

```cpp
#pragma once

#include "CommandQueue.h"
#include "../domain/RuntimeState.h"
#include "../domain/AppSettings.h"

namespace incubator::app
{
    class AppController
    {
    public:
        AppController(
            incubator::domain::RuntimeState& runtime,
            incubator::domain::AppSettings& settings,
            CommandQueue& queue);

    public:
        void tick();

    private:
        bool validate(const Command& cmd);
        void mutate(const Command& cmd);

    private:
        incubator::domain::RuntimeState& m_runtime;
        incubator::domain::AppSettings& m_settings;

        CommandQueue& m_queue;
    };
}
```

---

## AppController.cpp

```cpp
#include "AppController.h"

namespace incubator::app
{
    using namespace incubator::domain;

    AppController::AppController(
        RuntimeState& runtime,
        AppSettings& settings,
        CommandQueue& queue)
        :
        m_runtime(runtime),
        m_settings(settings),
        m_queue(queue)
    {
    }

    void AppController::tick()
    {
        Command cmd;

        while (m_queue.dequeue(cmd))
        {
            if (!validate(cmd))
            {
                continue;
            }

            mutate(cmd);
        }
    }

    bool AppController::validate(const Command& cmd)
    {
        if (m_runtime.safeMode)
        {
            switch (cmd.type)
            {
                case CommandType::ToggleManualHeater:
                case CommandType::ToggleManualHumidifier:
                    return false;

                default:
                    break;
            }
        }

        switch (cmd.type)
        {
            case CommandType::SetTargetTemperature:
            {
                const float v = cmd.setFloat.value;

                if (v < 30.0f || v > 40.0f)
                {
                    return false;
                }

                break;
            }

            case CommandType::SetTargetHumidity:
            {
                const float v = cmd.setFloat.value;

                if (v < 20.0f || v > 90.0f)
                {
                    return false;
                }

                break;
            }

            default:
                break;
        }

        return true;
    }

    void AppController::mutate(const Command& cmd)
    {
        switch (cmd.type)
        {
            case CommandType::StartBatch:
            {
                m_runtime.batchActive = true;
                m_runtime.mode = SystemMode::Running;
                break;
            }

            case CommandType::StopBatch:
            {
                m_runtime.batchActive = false;
                m_runtime.mode = SystemMode::Idle;
                break;
            }

            case CommandType::EnterSafeMode:
            {
                m_runtime.safeMode = true;
                m_runtime.mode = SystemMode::SafeMode;

                m_runtime.heaterOn = false;
                m_runtime.humidifierOn = false;
                m_runtime.turnerOn = false;
                m_runtime.fanPwm = 0;

                break;
            }

            case CommandType::ClearSafeMode:
            {
                if (m_runtime.sensorHealthy &&
                    m_runtime.storageHealthy)
                {
                    m_runtime.safeMode = false;
                    m_runtime.mode = SystemMode::Idle;
                }

                break;
            }

            case CommandType::SetTargetTemperature:
            {
                m_runtime.targetTempC = cmd.setFloat.value;
                break;
            }

            case CommandType::SetTargetHumidity:
            {
                m_runtime.targetHumidityPct = cmd.setFloat.value;
                break;
            }

            case CommandType::ToggleManualHeater:
            {
                m_runtime.heaterOn = cmd.toggle.enabled;
                break;
            }

            case CommandType::ToggleManualHumidifier:
            {
                m_runtime.humidifierOn = cmd.toggle.enabled;
                break;
            }

            default:
                break;
        }
    }
}
```

---

# 8. Main Loop 구조

## main.cpp

```cpp
#include <Arduino.h>

#include "domain/RuntimeState.h"
#include "domain/AppSettings.h"

#include "app/AppController.h"
#include "app/CommandQueue.h"

using namespace incubator;

static domain::RuntimeState g_runtime;
static domain::AppSettings g_settings;

static app::CommandQueue g_commandQueue;

static app::AppController g_appController(
    g_runtime,
    g_settings,
    g_commandQueue);

void setup()
{
    Serial.begin(115200);
}

void loop()
{
    g_runtime.uptimeMs = millis();

    // Sensor Tick

    // Scheduler Tick

    // Climate Tick

    // Alarm Tick

    g_appController.tick();

    // UI Tick

    // Cloud Tick
}
```

---

# 9. 현재 구조의 중요한 장점

## 1) 상태 흐름 추적 가능

```text
입력
 ↓
Command
 ↓
Queue
 ↓
Controller
 ↓
Mutation
```

즉시 이해 가능.

---

## 2) Race Condition 감소

Single Main Loop 기반.

---

## 3) SafeMode 보호 단순화

Validation 중앙 집중.

---

## 4) UI / Cloud 동등 구조

```text
UI
Cloud
BLE
Recovery
```

모두 동일한 Command 흐름 사용.

---

# 10. 다음 단계

다음 단계는:

```text
DDU-SENSOR-001
Sensor Pipeline
```

이다.

다음 구현 예정:

* SensorSample
* SensorManager
* Polling Tick
* RuntimeState Update
* Sensor Timeout
* Sensor Health
* Dirty Update
* Alarm Trigger 기반

핵심 목표:

```text
센서가 시스템 전체의 데이터 원천이 되는 구조
```
