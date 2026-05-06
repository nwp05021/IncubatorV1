
# DDU-IMPL-006 — CommandQueue Actual Source

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + PlatformIO

---

# 1. 목적

Heap 없는 고정 크기 CommandQueue 실제 구현.

목표:

- RingBuffer 기반
- Non-Blocking
- Predictable Memory
- Overflow 보호

---

# 2. 핵심 철학

```text
Queue는 단순해야 한다.
```

---

# 3. Command.h

```cpp
#pragma once

#include <stdint.h>

namespace incubator::app
{
    enum class CommandType
    {
        None,

        SetTargetTemperature,
        SetTargetHumidity,

        HeaterOn,
        HeaterOff,

        FanPwm,

        EnterSafeMode,
        ClearSafeMode
    };

    enum class CommandSource
    {
        UI,
        Cloud,
        BLE,
        Recovery,
        Internal
    };

    struct FloatPayload
    {
        float value = 0.0f;
    };

    struct UIntPayload
    {
        uint32_t value = 0;
    };

    struct Command
    {
        CommandType type =
            CommandType::None;

        CommandSource source =
            CommandSource::Internal;

        union
        {
            FloatPayload setFloat;

            UIntPayload setUInt;
        };
    };
}
```

---

# 4. CommandQueue.h

```cpp
#pragma once

#include "Command.h"

namespace incubator::app
{
    class CommandQueue
    {
    public:
        static constexpr uint16_t Capacity = 32;

    public:
        bool enqueue(
            const Command& cmd);

        bool dequeue(
            Command& cmd);

        uint16_t count() const;

        bool isFull() const;

        bool isEmpty() const;

    private:
        Command m_queue[Capacity];

        uint16_t m_head = 0;

        uint16_t m_tail = 0;

        uint16_t m_count = 0;
    };
}
```

---

# 5. CommandQueue.cpp

```cpp
#include "CommandQueue.h"

namespace incubator::app
{
    bool CommandQueue::enqueue(
        const Command& cmd)
    {
        if (m_count >= Capacity)
        {
            return false;
        }

        m_queue[m_tail] = cmd;

        m_tail =
            (m_tail + 1) % Capacity;

        ++m_count;

        return true;
    }

    bool CommandQueue::dequeue(
        Command& cmd)
    {
        if (m_count == 0)
        {
            return false;
        }

        cmd = m_queue[m_head];

        m_head =
            (m_head + 1) % Capacity;

        --m_count;

        return true;
    }

    uint16_t CommandQueue::count() const
    {
        return m_count;
    }

    bool CommandQueue::isFull() const
    {
        return m_count >= Capacity;
    }

    bool CommandQueue::isEmpty() const
    {
        return m_count == 0;
    }
}
```

---

# 6. Overflow 전략

## 핵심 원칙

```text
Overflow는 감춰지면 안 된다.
```

---

## 권장

```text
Event 생성
Alarm 가능
```

---

# 7. Queue 사용 흐름

```text
UI
Cloud
BLE
Recovery
    ↓
CommandQueue
    ↓
AppController
```

---

# 8. 금지 사항

```text
❌ dynamic allocation

❌ std::queue

❌ blocking wait

❌ infinite retry enqueue
```

---

# 9. Acceptance Criteria

```text
AC-1
FIFO 정상 동작

AC-2
Overflow 보호

AC-3
Heap 사용 없음

AC-4
dequeue 정상 동작

AC-5
isFull/isEmpty 정상
```
