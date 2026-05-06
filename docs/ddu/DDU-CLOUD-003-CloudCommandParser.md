
# DDU-CLOUD-003 — Cloud Command Parser

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + AWS IoT
> Dependency:
> - CommandQueue
> - AppController
> - TelemetryBuilder
>
> Estimated Time: 20~40 min

---

# 1. 목적

AWS desired 기반 Command Parsing 구조 구현.

핵심 흐름:

```text
AWS desired
    ↓
CmdParser
    ↓
Command
    ↓
CommandQueue
    ↓
AppController
```

목표:

- JSON → Command
- Queue Injection
- Delta Handling
- Validation 연계
- Cloud 비특권 구조 유지

---

# 2. 생성 파일

```text
product/cloud/CmdParser.h
product/cloud/CmdParser.cpp
```

---

# 3. 핵심 철학

```text
Cloud는 특권 계층이 아니다.
```

---

## 의미

Cloud도:

```text
UI
BLE
Recovery
```

와 동일한 Command 규칙 사용.

---

# 4. CmdParser 역할

```text
JSON Parse
    ↓
Command 생성
    ↓
Queue enqueue
```

---

## 금지

```text
❌ RuntimeState 직접 변경

❌ GPIO 직접 제어

❌ AppSettings 직접 저장
```

---

# 5. CmdParser.h

```cpp
#pragma once

#include "../app/CommandQueue.h"

namespace incubator::cloud
{
    class CmdParser
    {
    public:
        CmdParser(
            incubator::app::CommandQueue& queue);

    public:
        bool processJson(
            const char* json);

    private:
        bool enqueueSetTargetTemp(
            float value);

        bool enqueueSetTargetHumidity(
            float value);

    private:
        incubator::app::CommandQueue& m_queue;
    };
}
```

---

# 6. JSON 구조 예시

## Target Temp

```json
{
  "cmd": "SET_TARGET_TEMP",
  "value": 37.5
}
```

---

## Target Humidity

```json
{
  "cmd": "SET_TARGET_HUMIDITY",
  "value": 60.0
}
```

---

## SafeMode Clear

```json
{
  "cmd": "CLEAR_SAFEMODE"
}
```

---

# 7. CmdParser.cpp

```cpp
#include "CmdParser.h"

namespace incubator::cloud
{
    using namespace incubator::app;

    CmdParser::CmdParser(
        CommandQueue& queue)
        :
        m_queue(queue)
    {
    }

    bool CmdParser::processJson(
        const char* json)
    {
        // TODO:
        // StaticJsonDocument Parse

        // Example:
        // cmd = SET_TARGET_TEMP

        return enqueueSetTargetTemp(37.5f);
    }

    bool CmdParser::enqueueSetTargetTemp(
        float value)
    {
        Command cmd;

        cmd.type =
            CommandType::SetTargetTemperature;

        cmd.source =
            CommandSource::Cloud;

        cmd.setFloat.value =
            value;

        return m_queue.enqueue(cmd);
    }

    bool CmdParser::enqueueSetTargetHumidity(
        float value)
    {
        Command cmd;

        cmd.type =
            CommandType::SetTargetHumidity;

        cmd.source =
            CommandSource::Cloud;

        cmd.setFloat.value =
            value;

        return m_queue.enqueue(cmd);
    }
}
```

---

# 8. Delta Sync 전략

## 핵심 원칙

```text
desired → Command
```

---

## 금지

```text
❌ desired → RuntimeState direct mutation
```

---

# 9. Queue Injection 전략

## 핵심 목표

```text
Single Threaded State Mutation
```

---

## 구조

```text
Cloud Input
    ↓
Queue
    ↓
AppController
```

---

# 10. Validation 연계

## 핵심 원칙

```text
Validation은 AppController
```

---

## Cloud는:

```text
명령 생성만 수행
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

# 12. SafeMode 전략

## 정책

```text
SafeMode 시 위험 Command 거부
```

---

## 예시

```text
Heater ON
Humidifier ON
```

거부 가능.

---

# 13. 핵심 장점

## 1) Cloud 비특권 구조

Cloud와 UI 동일 규칙 유지.

---

## 2) Validation 중앙화

위험 제어 단순화.

---

## 3) Queue 기반 구조

Race 감소.

---

# 14. 금지 사항

```text
❌ Cloud direct GPIO

❌ RuntimeState direct mutation

❌ Blocking MQTT callback

❌ delay()
```

---

# 15. Acceptance Criteria

```text
AC-1
JSON Parse 정상 동작

AC-2
Command Queue enqueue 정상

AC-3
Cloud source 기록

AC-4
Validation AppController 유지

AC-5
Main Loop blocking 없음
```

---

# 16. 다음 단계

다음 DDU:

```text
DDU-UI-003
Alarm Overlay + SafeMode Overlay
```

다음 구현 예정:

- Alarm Overlay
- SafeMode Overlay
- Modal Priority
- Critical UI
- Dirty Overlay Render
