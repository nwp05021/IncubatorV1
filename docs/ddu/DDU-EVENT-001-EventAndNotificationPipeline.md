
# DDU-EVENT-001 — Event & Notification Pipeline

> Version: 1.0
> Status: Draft
> Target: ESP32-S3
> Dependency:
> - RuntimeState
> - AlarmState
> - CloudState
>
> Estimated Time: 20~40 min

---

# 1. 목적

상태 변화 기반 Event / Notification 구조 구현.

핵심 흐름:

```text
RuntimeState Change
    ↓
EventBuilder
    ↓
EventQueue
    ↓
Toast / Cloud / History
```

목표:

- Alarm Event
- Event History
- Toast Notification
- Critical Event Queue
- Cloud Notification

---

# 2. 생성 파일

```text
product/event/EventRecord.h

product/event/EventQueue.h
product/event/EventQueue.cpp

product/event/EventBuilder.h
product/event/EventBuilder.cpp

product/ui/toast/ToastManager.h
product/ui/toast/ToastManager.cpp
```

---

# 3. 핵심 철학

```text
Event는 상태 변경 결과를 기록하는 것이다.
```

---

## 금지

```text
❌ Event가 상태를 직접 변경

❌ Event가 GPIO 직접 제어
```

---

# 4. EventRecord

## 목적

표준 Event 데이터.

---

## EventRecord.h

```cpp
#pragma once

#include <stdint.h>

namespace incubator::event
{
    enum class EventLevel
    {
        Info,
        Warning,
        Alarm,
        Critical
    };

    struct EventRecord
    {
        uint32_t timestampMs = 0;

        EventLevel level =
            EventLevel::Info;

        char message[64] = {0};
    };
}
```

---

# 5. EventQueue

## 역할

```text
Event 저장
FIFO 관리
History 제공
```

---

## 핵심 원칙

```text
고정 크기 Queue
```

---

## EventQueue.h

```cpp
#pragma once

#include "EventRecord.h"

namespace incubator::event
{
    class EventQueue
    {
    public:
        static constexpr uint16_t Capacity = 64;

    public:
        bool push(
            const EventRecord& event);

        bool pop(
            EventRecord& event);

        uint16_t count() const;

    private:
        EventRecord m_events[Capacity];

        uint16_t m_head = 0;

        uint16_t m_tail = 0;

        uint16_t m_count = 0;
    };
}
```

---

# 6. EventQueue.cpp

```cpp
#include "EventQueue.h"

namespace incubator::event
{
    bool EventQueue::push(
        const EventRecord& event)
    {
        if (m_count >= Capacity)
        {
            return false;
        }

        m_events[m_tail] = event;

        m_tail =
            (m_tail + 1) % Capacity;

        ++m_count;

        return true;
    }

    bool EventQueue::pop(
        EventRecord& event)
    {
        if (m_count == 0)
        {
            return false;
        }

        event = m_events[m_head];

        m_head =
            (m_head + 1) % Capacity;

        --m_count;

        return true;
    }

    uint16_t EventQueue::count() const
    {
        return m_count;
    }
}
```

---

# 7. EventBuilder

## 역할

```text
상태 변화 감지
    ↓
Event 생성
```

---

## EventBuilder.h

```cpp
#pragma once

#include "../domain/RuntimeState.h"
#include "../domain/AlarmState.h"

#include "EventQueue.h"

namespace incubator::event
{
    class EventBuilder
    {
    public:
        EventBuilder(
            EventQueue& queue);

    public:
        void process(
            const incubator::domain::RuntimeState& runtime,
            const incubator::domain::AlarmState& alarm);

    private:
        void pushAlarmEvent(
            const char* message);

    private:
        EventQueue& m_queue;

        bool m_prevHighTemp = false;

        bool m_prevSafeMode = false;
    };
}
```

---

# 8. EventBuilder.cpp

```cpp
#include "EventBuilder.h"

#include <string.h>

namespace incubator::event
{
    using namespace incubator::domain;

    EventBuilder::EventBuilder(
        EventQueue& queue)
        :
        m_queue(queue)
    {
    }

    void EventBuilder::process(
        const RuntimeState& runtime,
        const AlarmState& alarm)
    {
        if (alarm.highTemp &&
            !m_prevHighTemp)
        {
            pushAlarmEvent(
                "HIGH TEMP");
        }

        if (runtime.safeMode &&
            !m_prevSafeMode)
        {
            pushAlarmEvent(
                "SAFE MODE");
        }

        m_prevHighTemp =
            alarm.highTemp;

        m_prevSafeMode =
            runtime.safeMode;
    }

    void EventBuilder::pushAlarmEvent(
        const char* message)
    {
        EventRecord event;

        event.level =
            EventLevel::Alarm;

        strncpy(
            event.message,
            message,
            sizeof(event.message) - 1);

        m_queue.push(event);
    }
}
```

---

# 9. ToastManager

## 역할

```text
짧은 상태 알림 표시
```

---

## 표시 예시

```text
WIFI CONNECTED

SETTINGS SAVED

SAFE MODE
```

---

## ToastManager.h

```cpp
#pragma once

#include "../event/EventRecord.h"

namespace incubator::ui
{
    class ToastManager
    {
    public:
        void push(
            const incubator::event::EventRecord& event);

        void tick(uint32_t nowMs);
    };
}
```

---

# 10. Event History 전략

## 목적

과거 문제 추적.

---

## 정책

```text
FIFO
고정 크기
오래된 Event 제거
```

---

# 11. Critical Event 전략

## Critical 예시

```text
SAFE MODE
SENSOR FAIL
STORAGE FAIL
```

---

## 정책

```text
Cloud 즉시 Publish
```

권장.

---

# 12. Main Event Flow

```text
RuntimeState
    ↓
EventBuilder
    ↓
EventQueue
    ↓
Toast
Cloud
History
```

---

# 13. Main Loop 연결

## main.cpp

```cpp
void loop()
{
    const uint32_t now = millis();

    diagnostics.tick(now);

    sensorManager.tick(now);

    scheduler.tick(now);

    climate.tick(now);

    turning.tick(now);

    fan.tick(now);

    alarm.tick(now);

    recovery.tick(now);

    appController.tick();

    eventBuilder.process(
        runtime,
        alarmState);

    toast.tick(now);

    ui.tick(now);

    wifi.tick(now);

    aws.tick(now);

    shadow.tick(now);
}
```

---

# 14. 핵심 장점

## 1) 상태 변화 기반 Event

중복 Event 감소.

---

## 2) Queue 기반 구조

Race 감소.

---

## 3) RuntimeState 중심 구조

Cloud/UI/Event 동일 데이터 사용.

---

# 15. 금지 사항

```text
❌ Event direct Runtime mutation

❌ Event direct GPIO

❌ Infinite Event loop

❌ Dynamic memory queue growth
```

---

# 16. Acceptance Criteria

```text
AC-1
Alarm Event 정상 생성

AC-2
SafeMode Event 정상 생성

AC-3
Toast 표시 정상 동작

AC-4
Event Queue 정상 동작

AC-5
History FIFO 정상 유지
```

---

# 17. 다음 단계

다음 DDU:

```text
DDU-TIME-001
RTC + NTP Time Pipeline
```

다음 구현 예정:

- RTC Restore
- NTP Sync
- Offline Time
- Epoch Tracking
- Time Service
