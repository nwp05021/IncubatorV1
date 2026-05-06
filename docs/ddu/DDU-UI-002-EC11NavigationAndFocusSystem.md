
# DDU-UI-002 — EC11 Navigation & Focus System

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + EC11
> Dependency:
> - Premium Home UI
> - RuntimeState
>
> Estimated Time: 20~40 min

---

# 1. 목적

EC11 기반 Premium Navigation 구조 구현.

핵심 흐름:

```text
EC11
    ↓
UiInputController
    ↓
UiNavigator
    ↓
UiStateMachine
    ↓
Renderer
```

목표:

- Rotary Navigation
- Focus System
- Overlay Navigation
- Long Hold Confirm
- Premium Embedded UX

---

# 2. 생성 파일

```text
product/ui/input/InputEvent.h

product/ui/input/UiInputController.h
product/ui/input/UiInputController.cpp

product/ui/navigation/UiNavigator.h
product/ui/navigation/UiNavigator.cpp

product/ui/navigation/UiStateMachine.h
product/ui/navigation/UiStateMachine.cpp
```

---

# 3. 핵심 철학

```text
입력은 단순해야 하고
현재 위치는 명확해야 한다.
```

---

# 4. 입력 모델

## EC11 입력

| 입력 | 의미 |
|---|---|
| Rotate Left | 이전 |
| Rotate Right | 다음 |
| Click | 선택 |
| Long Click | Back |
| Long Hold | 위험 승인 |

---

# 5. InputEvent

## 역할

모든 입력의 표준 구조.

---

## InputEvent.h

```cpp
#pragma once

namespace incubator::ui
{
    enum class InputEventType
    {
        None,

        RotateLeft,
        RotateRight,

        Click,
        LongClick,
        LongHold
    };

    struct InputEvent
    {
        InputEventType type =
            InputEventType::None;
    };
}
```

---

# 6. UiInputController

## 역할

```text
GPIO 입력
    ↓
InputEvent 변환
```

---

## 금지

```text
❌ Navigation 직접 처리

❌ Renderer 접근

❌ RuntimeState 변경
```

---

## UiInputController.h

```cpp
#pragma once

#include "InputEvent.h"

namespace incubator::ui
{
    class UiInputController
    {
    public:
        bool poll(InputEvent& event);
    };
}
```

---

# 7. UiStateMachine

## 역할

현재 UI 상태 관리.

---

## UI 상태

```text
Home
Menu
Edit
Dialog
AlarmOverlay
SafeModeOverlay
```

---

## UiStateMachine.h

```cpp
#pragma once

namespace incubator::ui
{
    enum class UiPage
    {
        Home,
        Progress,
        Manual,
        PlanEdit,
        System
    };

    enum class UiOverlay
    {
        None,
        Dialog,
        Alarm,
        SafeMode
    };

    class UiStateMachine
    {
    public:
        UiPage currentPage =
            UiPage::Home;

        UiOverlay currentOverlay =
            UiOverlay::None;
    };
}
```

---

# 8. UiNavigator

## 역할

```text
InputEvent
    ↓
UI 상태 전환
```

---

## UiNavigator.h

```cpp
#pragma once

#include "../input/InputEvent.h"
#include "UiStateMachine.h"

namespace incubator::ui
{
    class UiNavigator
    {
    public:
        UiNavigator(
            UiStateMachine& state);

    public:
        void process(
            const InputEvent& event);

    private:
        void navigateNext();

        void navigatePrev();

        void select();

        void back();

    private:
        UiStateMachine& m_state;
    };
}
```

---

# 9. UiNavigator.cpp

```cpp
#include "UiNavigator.h"

namespace incubator::ui
{
    UiNavigator::UiNavigator(
        UiStateMachine& state)
        :
        m_state(state)
    {
    }

    void UiNavigator::process(
        const InputEvent& event)
    {
        switch (event.type)
        {
            case InputEventType::RotateRight:
                navigateNext();
                break;

            case InputEventType::RotateLeft:
                navigatePrev();
                break;

            case InputEventType::Click:
                select();
                break;

            case InputEventType::LongClick:
                back();
                break;

            default:
                break;
        }
    }

    void UiNavigator::navigateNext()
    {
        switch (m_state.currentPage)
        {
            case UiPage::Home:
                m_state.currentPage =
                    UiPage::Progress;
                break;

            case UiPage::Progress:
                m_state.currentPage =
                    UiPage::Manual;
                break;

            case UiPage::Manual:
                m_state.currentPage =
                    UiPage::PlanEdit;
                break;

            case UiPage::PlanEdit:
                m_state.currentPage =
                    UiPage::System;
                break;

            default:
                break;
        }
    }

    void UiNavigator::navigatePrev()
    {
    }

    void UiNavigator::select()
    {
    }

    void UiNavigator::back()
    {
        m_state.currentOverlay =
            UiOverlay::None;
    }
}
```

---

# 10. Focus System

## 목적

현재 조작 위치 명확화.

---

## 권장 표현

```text
Border
Underline
Reverse Color
```

---

## 금지

```text
❌ RGB Flash

❌ 빠른 깜빡임

❌ 과한 애니메이션
```

---

# 11. Overlay 우선순위

```text
SafeMode
    > Alarm
        > Dialog
            > Normal UI
```

---

# 12. Long Hold 정책

## 위험 기능

```text
Heater ON
Factory Reset
SafeMode Clear
```

---

## 승인 방식

```text
Long Hold Only
```

---

# 13. Dirty Navigation

## 핵심 철학

```text
필요한 부분만 redraw
```

---

## 예시

```text
Focus 이동
    ↓
Focus 영역만 redraw
```

---

# 14. Main UI Flow

```text
Input
    ↓
Navigator
    ↓
UiState
    ↓
Renderer
```

---

# 15. 핵심 장점

## 1) 입력/렌더 분리

입력과 Draw 완전 분리.

---

## 2) Overlay 구조 단순화

```text
UiOverlay
```

한 곳에서 관리.

---

## 3) 유지보수 용이

Page 추가 단순.

---

# 16. 금지 사항

```text
❌ Input 내부 Renderer 접근

❌ Navigator 내부 GPIO 접근

❌ UI direct RuntimeState mutation

❌ Overlay 내부 business logic
```

---

# 17. Acceptance Criteria

```text
AC-1
Rotate 입력 정상 동작

AC-2
Page Navigation 정상 동작

AC-3
Overlay 우선순위 유지

AC-4
Focus 이동 redraw 최소화

AC-5
Long Hold 구조 분리
```

---

# 18. 다음 단계

다음 DDU:

```text
DDU-CLOUD-001
WiFi + MQTT Foundation
```

다음 구현 예정:

- WifiManager
- AwsIotClient
- MQTT Retry
- Offline First
- Cloud State
