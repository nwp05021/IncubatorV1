
# DDU-ALARM-001 — Alarm Pipeline

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + PlatformIO
> Dependency:
> - RuntimeState
> - AppController
> - Scheduler
> - Sensor Pipeline
>
> Estimated Time: 20~40 min

---

# 1. 목적

RuntimeState 기반 Alarm 감시 및 SafeMode 연동.

핵심 흐름:

```text
RuntimeState
    ↓
AlarmModule
    ↓
AlarmState
    ↓
UI Overlay
    ↓
Cloud Publish
```

목표:

- 고온 감지
- 저온 감지
- 센서 오류 감지
- Alarm Delay
- SafeMode Trigger

---

# 2. 생성 파일

```text
product/domain/AlarmState.h

product/modules/alarm/AlarmModule.h
product/modules/alarm/AlarmModule.cpp
```

---

# 3. 핵심 철학

Alarm은:

```text
사용자가 행동해야 하는 상태
```

를 의미한다.

---

## 금지

```text
❌ Alarm 내부 UI Draw

❌ Alarm 내부 MQTT Publish

❌ Alarm 내부 Storage 접근
```

---

# 4. AlarmState

## 역할

현재 Alarm 상태 관리.

---

## AlarmState.h

```cpp
#pragma once

namespace incubator::domain
{
    enum class AlarmLevel
    {
        None,
        Info,
        Warning,
        Alarm,
        Critical
    };

    struct AlarmState
    {
        bool highTemp = false;

        bool lowTemp = false;

        bool sensorFail = false;

        AlarmLevel level = AlarmLevel::None;
    };
}
```

---

# 5. Alarm 정책

## High Temp

```text
현재 온도 > 목표 + 1.5°C
```

---

## Low Temp

```text
현재 온도 < 목표 - 1.5°C
```

---

## Sensor Fail

```text
sensorHealthy == false
```

---

# 6. Alarm Delay

## 목적

짧은 노이즈 무시.

---

## 정책

| Alarm | Delay |
|---|---|
| HighTemp | 10 sec |
| LowTemp | 10 sec |
| SensorFail | Immediate |

---

# 7. AlarmModule

## AlarmModule.h

```cpp
#pragma once

#include "../../domain/RuntimeState.h"
#include "../../domain/AlarmState.h"

namespace incubator::modules::alarm
{
    class AlarmModule
    {
    public:
        AlarmModule(
            incubator::domain::RuntimeState& runtime,
            incubator::domain::AlarmState& alarm);

    public:
        void tick(uint32_t nowMs);

    private:
        void processTemperatureAlarm(uint32_t nowMs);

        void processSensorAlarm();

        void updateAlarmLevel();

    private:
        incubator::domain::RuntimeState& m_runtime;

        incubator::domain::AlarmState& m_alarm;

        uint32_t m_highTempStartMs = 0;

        uint32_t m_lowTempStartMs = 0;

        uint32_t m_lastTickMs = 0;

        static constexpr uint32_t TickIntervalMs = 1000;

        static constexpr float TempAlarmDelta = 1.5f;

        static constexpr uint32_t AlarmDelayMs = 10000;
    };
}
```

---

# 8. AlarmModule.cpp

```cpp
#include "AlarmModule.h"

namespace incubator::modules::alarm
{
    using namespace incubator::domain;

    AlarmModule::AlarmModule(
        RuntimeState& runtime,
        AlarmState& alarm)
        :
        m_runtime(runtime),
        m_alarm(alarm)
    {
    }

    void AlarmModule::tick(uint32_t nowMs)
    {
        if ((nowMs - m_lastTickMs) < TickIntervalMs)
        {
            return;
        }

        m_lastTickMs = nowMs;

        processTemperatureAlarm(nowMs);

        processSensorAlarm();

        updateAlarmLevel();
    }

    void AlarmModule::processTemperatureAlarm(uint32_t nowMs)
    {
        const float highThreshold =
            m_runtime.targetTempC + TempAlarmDelta;

        const float lowThreshold =
            m_runtime.targetTempC - TempAlarmDelta;

        if (m_runtime.currentTempC > highThreshold)
        {
            if (m_highTempStartMs == 0)
            {
                m_highTempStartMs = nowMs;
            }

            if ((nowMs - m_highTempStartMs) >= AlarmDelayMs)
            {
                m_alarm.highTemp = true;

                m_runtime.highTempAlarm = true;
            }
        }
        else
        {
            m_highTempStartMs = 0;

            m_alarm.highTemp = false;

            m_runtime.highTempAlarm = false;
        }

        if (m_runtime.currentTempC < lowThreshold)
        {
            if (m_lowTempStartMs == 0)
            {
                m_lowTempStartMs = nowMs;
            }

            if ((nowMs - m_lowTempStartMs) >= AlarmDelayMs)
            {
                m_alarm.lowTemp = true;

                m_runtime.lowTempAlarm = true;
            }
        }
        else
        {
            m_lowTempStartMs = 0;

            m_alarm.lowTemp = false;

            m_runtime.lowTempAlarm = false;
        }
    }

    void AlarmModule::processSensorAlarm()
    {
        if (!m_runtime.sensorHealthy)
        {
            m_alarm.sensorFail = true;

            m_runtime.sensorFailAlarm = true;

            m_runtime.safeMode = true;
        }
        else
        {
            m_alarm.sensorFail = false;

            m_runtime.sensorFailAlarm = false;
        }
    }

    void AlarmModule::updateAlarmLevel()
    {
        m_alarm.level = AlarmLevel::None;

        if (m_alarm.highTemp ||
            m_alarm.lowTemp)
        {
            m_alarm.level = AlarmLevel::Alarm;
        }

        if (m_alarm.sensorFail)
        {
            m_alarm.level = AlarmLevel::Critical;
        }
    }
}
```

---

# 9. SafeMode 전략

## 핵심 원칙

```text
의심되면 안전 정지
```

---

## Sensor Fail

```text
sensorHealthy == false
    ↓
safeMode = true
```

---

# 10. Main Loop 연결

## main.cpp

```cpp
void loop()
{
    const uint32_t now = millis();

    sensorManager.tick(now);

    scheduler.tick(now);

    climate.tick(now);

    alarm.tick(now);

    appController.tick();

    ui.tick(now);

    cloud.tick(now);
}
```

---

# 11. Alarm Overlay 연결

UI는:

```text
AlarmState
```

만 읽는다.

---

## 금지

```text
❌ UI 직접 Alarm 판단
```

---

# 12. 핵심 장점

## 1) Alarm 정책 중앙화

Alarm 조건을 한 곳에서 관리.

---

## 2) RuntimeState 기반 구조

Cloud/UI 동일 데이터 사용.

---

## 3) SafeMode 연계 단순화

```text
Alarm
    ↓
SafeMode
```

흐름 명확.

---

# 13. 금지 사항

```text
❌ Alarm 내부 GPIO 제어

❌ Alarm 내부 Display Draw

❌ Alarm 내부 MQTT Publish

❌ Alarm 내부 delay()
```

---

# 14. Acceptance Criteria

```text
AC-1
고온 Alarm 정상 발생

AC-2
저온 Alarm 정상 발생

AC-3
10초 Delay 정상 동작

AC-4
Sensor Fail 시 SafeMode 진입

AC-5
RuntimeState Alarm 반영
```

---

# 15. 다음 단계

다음 DDU:

```text
DDU-REC-001
Recovery & SafeMode Manager
```

다음 구현 예정:

- Boot Recovery
- Reset Reason
- SafeMode Manager
- Recovery Flags
- Restore Flow
- Factory Reset
