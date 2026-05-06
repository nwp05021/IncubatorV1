
# DDU-UI-003 — Alarm Overlay + SafeMode Overlay

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + ST7789
> Dependency:
> - AlarmState
> - RuntimeState
> - UiStateMachine
>
> Estimated Time: 20~40 min

---

# 1. 목적

Critical Overlay 기반 Premium Industrial UX 구현.

핵심 흐름:

```text
AlarmState
    ↓
OverlayManager
    ↓
Alarm Overlay
    ↓
SafeMode Overlay
```

목표:

- Alarm Overlay
- SafeMode Overlay
- Overlay Priority
- Dirty Overlay Render
- Critical 상태 즉시 인지

---

# 2. 생성 파일

```text
product/ui/overlay/OverlayManager.h
product/ui/overlay/OverlayManager.cpp

product/ui/overlay/AlarmOverlayRenderer.h
product/ui/overlay/AlarmOverlayRenderer.cpp

product/ui/overlay/SafeModeOverlayRenderer.h
product/ui/overlay/SafeModeOverlayRenderer.cpp
```

---

# 3. 핵심 철학

```text
Alarm은 로그가 아니라
사용자 행동을 유도해야 한다.
```

---

# 4. Overlay 우선순위

```text
SafeMode
    > Alarm
        > Dialog
            > Normal UI
```

---

# 5. OverlayManager

## 역할

```text
현재 Overlay 결정
우선순위 관리
Dirty Overlay 판단
```

---

## OverlayManager.h

```cpp
#pragma once

#include "../../domain/RuntimeState.h"
#include "../../domain/AlarmState.h"

namespace incubator::ui
{
    enum class OverlayType
    {
        None,
        Dialog,
        Alarm,
        SafeMode
    };

    class OverlayManager
    {
    public:
        OverlayType resolve(
            const incubator::domain::RuntimeState& runtime,
            const incubator::domain::AlarmState& alarm);
    };
}
```

---

# 6. OverlayManager.cpp

```cpp
#include "OverlayManager.h"

namespace incubator::ui
{
    using namespace incubator::domain;

    OverlayType OverlayManager::resolve(
        const RuntimeState& runtime,
        const AlarmState& alarm)
    {
        if (runtime.safeMode)
        {
            return OverlayType::SafeMode;
        }

        if (alarm.level == AlarmLevel::Alarm ||
            alarm.level == AlarmLevel::Critical)
        {
            return OverlayType::Alarm;
        }

        return OverlayType::None;
    }
}
```

---

# 7. Alarm Overlay

## 목적

즉시 위험 상태 표시.

---

## 표시 예시

```text
HIGH TEMP
39.2°C
CHECK HEATER
```

---

## AlarmOverlayRenderer.h

```cpp
#pragma once

#include "../../domain/AlarmState.h"

namespace incubator::ui
{
    class AlarmOverlayRenderer
    {
    public:
        void render(
            const incubator::domain::AlarmState& alarm);
    };
}
```

---

# 8. SafeMode Overlay

## 목적

치명 상태 보호 표시.

---

## 표시 예시

```text
SAFE MODE

OUTPUTS DISABLED

CHECK SENSOR
```

---

## SafeModeOverlayRenderer.h

```cpp
#pragma once

#include "../../domain/RecoveryState.h"

namespace incubator::ui
{
    class SafeModeOverlayRenderer
    {
    public:
        void render(
            incubator::domain::SafeModeReason reason);
    };
}
```

---

# 9. Dirty Overlay Render

## 핵심 철학

```text
Overlay 변경 시만 redraw
```

---

## 전략

```text
OverlayType 변경
    ↓
Overlay redraw
```

---

## 금지

```text
❌ Full Screen redraw 반복
```

---

# 10. 색상 정책

| 상태 | 색상 |
|---|---|
| Warning | Yellow |
| Alarm | Red |
| SafeMode | Deep Red |

---

# 11. Animation 정책

## 허용

```text
Soft Fade
Gentle Highlight
```

---

## 금지

```text
❌ RGB Flash

❌ Fast Blink

❌ Bounce Animation
```

---

# 12. Main UI Flow

```text
RuntimeState
    ↓
AlarmState
    ↓
OverlayManager
    ↓
OverlayRenderer
```

---

# 13. Main Loop 연결

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

# 14. SafeMode UX

## 핵심 목표

```text
사용자가 즉시 위험을 이해
```

---

## 표시 원칙

```text
간단하게
크게
명확하게
```

---

# 15. 핵심 장점

## 1) Overlay 중앙화

우선순위 충돌 제거.

---

## 2) Premium Industrial UX

Critical 상태 즉시 인지.

---

## 3) Dirty Render 유지

성능 안정성 향상.

---

# 16. 금지 사항

```text
❌ Overlay 내부 상태 변경

❌ Overlay 내부 GPIO 제어

❌ Overlay 내부 MQTT 처리

❌ Full redraw 남발
```

---

# 17. Acceptance Criteria

```text
AC-1
Alarm Overlay 정상 표시

AC-2
SafeMode Overlay 최우선 표시

AC-3
Overlay Priority 유지

AC-4
Dirty Overlay Render 정상 동작

AC-5
Critical 상태 즉시 인지 가능
```

---

# 18. 다음 단계

다음 DDU:

```text
DDU-UI-004
Progress Screen + Trend UI
```

다음 구현 예정:

- Progress Screen
- Day ProgressBar
- Trend Graph
- Mini Graph
- Ambient UI
