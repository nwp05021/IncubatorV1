
# DDU-IMPL-005 — AppController Actual Source

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + PlatformIO

---

# 1. 목적

유일한 상태 변경 지점(AppController)을 실제 구현 수준으로 정의한다.

---

# 2. 핵심 철학

```text
모든 상태 변경은
AppController만 수행한다.
```

---

# 3. AppController.h

```cpp
#pragma once

#include "../domain/RuntimeState.h"
#include "../domain/AppSettings.h"

#include "CommandQueue.h"

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
        void processCommand(
            const Command& cmd);

        void handleSetTargetTemp(
            const Command& cmd);

        void handleSetTargetHumidity(
            const Command& cmd);

    private:
        incubator::domain::RuntimeState& m_runtime;

        incubator::domain::AppSettings& m_settings;

        CommandQueue& m_queue;
    };
}
```

---

# 4. AppController.cpp

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
            processCommand(cmd);
        }
    }

    void AppController::processCommand(
        const Command& cmd)
    {
        switch (cmd.type)
        {
            case CommandType::SetTargetTemperature:
                handleSetTargetTemp(cmd);
                break;

            case CommandType::SetTargetHumidity:
                handleSetTargetHumidity(cmd);
                break;

            default:
                break;
        }
    }

    void AppController::handleSetTargetTemp(
        const Command& cmd)
    {
        const float value =
            cmd.setFloat.value;

        if (value < 30.0f ||
            value > 40.0f)
        {
            return;
        }

        m_runtime.targetTempC =
            value;
    }

    void AppController::handleSetTargetHumidity(
        const Command& cmd)
    {
        const float value =
            cmd.setFloat.value;

        if (value < 20.0f ||
            value > 90.0f)
        {
            return;
        }

        m_runtime.targetHumidityPct =
            value;
    }
}
```

---

# 5. 핵심 규칙

```text
Validation은
AppController 내부에서 수행
```

---

# 6. 금지 사항

```text
❌ UI direct RuntimeState mutation

❌ Cloud direct mutation

❌ Device direct mutation
```

---

# 7. Acceptance Criteria

```text
AC-1
Queue 기반 처리

AC-2
Validation 정상 동작

AC-3
Invalid 값 거부

AC-4
RuntimeState 변경 정상

AC-5
Single mutation point 유지
```
