
# DDU-UI-005 — Manual Control + Safety UX

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + EC11 + ST7789
> Dependency:
> - AppController
> - RuntimeState
> - UiNavigator
>
> Estimated Time: 20~40 min

---

# 1. 목적

위험 출력 제어를 위한 Premium Safety UX 구현.

핵심 흐름:

```text
User Input
    ↓
Command
    ↓
AppController
    ↓
RuntimeState
```

목표:

- Manual Mode
- Heater Manual Control
- Fan Manual PWM
- Long Hold Confirm
- Dangerous Action UX

---

# 2. 생성 파일

```text
product/ui/model/ManualUiModel.h

product/ui/viewmodel/ManualUiModelBuilder.h
product/ui/viewmodel/ManualUiModelBuilder.cpp

product/ui/render/ManualRenderer.h
product/ui/render/ManualRenderer.cpp

product/ui/dialog/HoldConfirmDialog.h
product/ui/dialog/HoldConfirmDialog.cpp
```

---

# 3. 핵심 철학

```text
위험 기능은 빠르게 실행되면 안 된다.
```

---

# 4. ManualUiModel

## 목적

Manual Screen 전용 ViewModel.

---

## ManualUiModel.h

```cpp
#pragma once

#include <stdint.h>

namespace incubator::ui
{
    struct ManualUiModel
    {
        bool heaterOn = false;

        bool humidifierOn = false;

        bool turnerOn = false;

        uint8_t fanPwm = 0;

        bool safeMode = false;

        bool manualMode = false;
    };
}
```

---

# 5. ManualUiModelBuilder

## 역할

```text
RuntimeState
    ↓
ManualUiModel
```

---

## ManualUiModelBuilder.h

```cpp
#pragma once

#include "../../domain/RuntimeState.h"
#include "../model/ManualUiModel.h"

namespace incubator::ui
{
    class ManualUiModelBuilder
    {
    public:
        void build(
            const incubator::domain::RuntimeState& runtime,
            ManualUiModel& model);
    };
}
```

---

## ManualUiModelBuilder.cpp

```cpp
#include "ManualUiModelBuilder.h"

namespace incubator::ui
{
    using namespace incubator::domain;

    void ManualUiModelBuilder::build(
        const RuntimeState& runtime,
        ManualUiModel& model)
    {
        model.heaterOn =
            runtime.heaterOn;

        model.humidifierOn =
            runtime.humidifierOn;

        model.turnerOn =
            runtime.turnerOn;

        model.fanPwm =
            runtime.fanPwm;

        model.safeMode =
            runtime.safeMode;

        model.manualMode =
            (runtime.mode ==
             SystemMode::Manual);
    }
}
```

---

# 6. ManualRenderer

## 역할

```text
Draw Only
```

---

## 표시 항목

```text
MANUAL MODE
HEATER
HUMIDIFIER
TURNER
FAN PWM
```

---

## ManualRenderer.h

```cpp
#pragma once

#include "../model/ManualUiModel.h"

namespace incubator::ui
{
    class ManualRenderer
    {
    public:
        void render(
            const ManualUiModel& model);
    };
}
```

---

# 7. Dangerous Action UX

## 대상 기능

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

# 8. HoldConfirmDialog

## 목적

위험 작업 보호.

---

## UX 예시

```text
HOLD TO ENABLE
HEATER
```

---

## HoldConfirmDialog.h

```cpp
#pragma once

namespace incubator::ui
{
    class HoldConfirmDialog
    {
    public:
        void open();

        void close();

        bool update(
            bool holding,
            uint32_t elapsedMs);

    private:
        bool m_opened = false;

        static constexpr uint32_t HoldMs =
            3000;
    };
}
```

---

# 9. Safety 정책

## SafeMode

```text
위험 출력 강제 차단
```

---

## Manual Mode

```text
명확히 표시
```

---

## 표시 예시

```text
MANUAL MODE ACTIVE
```

---

# 10. Command 흐름

## 핵심 구조

```text
UI Input
    ↓
Command
    ↓
Queue
    ↓
AppController
```

---

## 금지

```text
❌ UI direct GPIO
```

---

# 11. Focus UX

## 현재 선택 위치

```text
Border
Reverse Color
```

---

## 금지

```text
❌ RGB Flash

❌ 빠른 Blink
```

---

# 12. Dirty Render 전략

## 핵심 철학

```text
상태 변경 시만 redraw
```

---

## 예시

```text
Fan PWM 변경
    ↓
PWM 영역 redraw
```

---

# 13. Main UI Flow

```text
RuntimeState
    ↓
ManualUiModelBuilder
    ↓
ManualUiModel
    ↓
ManualRenderer
```

---

# 14. 핵심 장점

## 1) 위험 작업 보호

실수 방지 UX 확보.

---

## 2) AppController 중심 구조

상태 변경 중앙화 유지.

---

## 3) Premium Safety UX

상용 장비 느낌 강화.

---

# 15. 금지 사항

```text
❌ UI direct RuntimeState mutation

❌ Renderer 내부 상태 계산

❌ GPIO direct control

❌ Dangerous Action 즉시 실행
```

---

# 16. Acceptance Criteria

```text
AC-1
Manual Screen 정상 표시

AC-2
Long Hold Confirm 정상 동작

AC-3
SafeMode 시 위험 기능 차단

AC-4
Manual Mode 명확히 표시

AC-5
Command 기반 구조 유지
```

---

# 17. 다음 단계

다음 DDU:

```text
DDU-PLAN-001
Plan Editor + Day Policy UX
```

다음 구현 예정:

- Day Plan Edit
- Temp/Humidity Edit
- Turning Interval Edit
- Validation UX
- Save Flow
