
# DDU-PLAN-001 — Plan Editor + Day Policy UX

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + EC11 + ST7789
> Dependency:
> - IncubationPlanTable
> - Scheduler
> - AppController
>
> Estimated Time: 20~40 min

---

# 1. 목적

Day 기반 부화 정책 편집 UX 구현.

핵심 흐름:

```text
User Edit
    ↓
Command
    ↓
AppController
    ↓
PlanTable
```

목표:

- Day Policy Edit
- Temp/Humidity Edit
- Turning Interval Edit
- Validation UX
- Save Flow

---

# 2. 생성 파일

```text
product/ui/model/PlanEditorUiModel.h

product/ui/viewmodel/PlanEditorUiModelBuilder.h
product/ui/viewmodel/PlanEditorUiModelBuilder.cpp

product/ui/render/PlanEditorRenderer.h
product/ui/render/PlanEditorRenderer.cpp

product/ui/editor/ValueEditor.h
product/ui/editor/ValueEditor.cpp
```

---

# 3. 핵심 철학

```text
설정 편집은 명확해야 하고
실수하기 어려워야 한다.
```

---

# 4. PlanEditorUiModel

## 목적

Plan Editor 전용 ViewModel.

---

## 표시 항목

```text
Day
Target Temp
Target Humidity
Turning Enabled
Turning Interval
```

---

## PlanEditorUiModel.h

```cpp
#pragma once

#include <stdint.h>

namespace incubator::ui
{
    struct PlanEditorUiModel
    {
        uint16_t day = 1;

        float targetTempC = 37.5f;

        float targetHumidityPct = 60.0f;

        bool turningEnabled = true;

        uint32_t turningIntervalMs = 7200000;

        bool modified = false;
    };
}
```

---

# 5. PlanEditorUiModelBuilder

## 역할

```text
PlanRow
    ↓
UiModel
```

---

## PlanEditorUiModelBuilder.h

```cpp
#pragma once

#include "../../domain/PlanRow.h"
#include "../model/PlanEditorUiModel.h"

namespace incubator::ui
{
    class PlanEditorUiModelBuilder
    {
    public:
        void build(
            const incubator::domain::PlanRow& row,
            PlanEditorUiModel& model);
    };
}
```

---

## PlanEditorUiModelBuilder.cpp

```cpp
#include "PlanEditorUiModelBuilder.h"

namespace incubator::ui
{
    using namespace incubator::domain;

    void PlanEditorUiModelBuilder::build(
        const PlanRow& row,
        PlanEditorUiModel& model)
    {
        model.day =
            row.day;

        model.targetTempC =
            row.targetTempC;

        model.targetHumidityPct =
            row.targetHumidityPct;

        model.turningEnabled =
            row.turningEnabled;

        model.turningIntervalMs =
            row.turningIntervalMs;
    }
}
```

---

# 6. ValueEditor

## 역할

값 편집 UX 제공.

---

## 기능

```text
Increment
Decrement
Range Validation
Step Apply
```

---

## ValueEditor.h

```cpp
#pragma once

namespace incubator::ui
{
    class ValueEditor
    {
    public:
        float increase(
            float value,
            float step,
            float max);

        float decrease(
            float value,
            float step,
            float min);
    };
}
```

---

# 7. Validation 정책

| 항목 | 범위 |
|---|---|
| Temp | 30~40 |
| Humidity | 20~90 |
| Turning Interval | 1min~24h |

---

# 8. PlanEditorRenderer

## 역할

```text
Draw Only
```

---

## 표시 예시

```text
DAY 07

TEMP      37.5°C
HUMIDITY  60%
TURNING   2h
```

---

## PlanEditorRenderer.h

```cpp
#pragma once

#include "../model/PlanEditorUiModel.h"

namespace incubator::ui
{
    class PlanEditorRenderer
    {
    public:
        void render(
            const PlanEditorUiModel& model);
    };
}
```

---

# 9. Editing UX

## 핵심 목표

```text
현재 편집 중인 값이 명확해야 한다.
```

---

## 권장

```text
Underline
Reverse Color
Focus Border
```

---

## 금지

```text
❌ RGB Flash

❌ 빠른 Blink

❌ 과한 Animation
```

---

# 10. Save Flow

## 핵심 구조

```text
Edit
    ↓
Command
    ↓
AppController
    ↓
PlanTable Update
    ↓
Storage Save
```

---

## 금지

```text
❌ UI direct save
```

---

# 11. Dirty Edit 표시

## 표시 예시

```text
UNSAVED *
```

---

## 목적

사용자 인지 강화.

---

# 12. Long Hold Save

## 위험 정책 변경

```text
Long Hold Save
```

권장.

---

# 13. Main UI Flow

```text
PlanRow
    ↓
UiModelBuilder
    ↓
UiModel
    ↓
Renderer
```

---

# 14. 핵심 장점

## 1) Plan 구조 독립성

Scheduler와 UI 분리.

---

## 2) Validation UX 강화

잘못된 값 사전 차단.

---

## 3) Premium Editing UX

상용 장비 느낌 강화.

---

# 15. 금지 사항

```text
❌ UI direct Plan mutation

❌ Renderer 내부 상태 계산

❌ Direct NVS save

❌ Runtime dynamic vector growth
```

---

# 16. Acceptance Criteria

```text
AC-1
Day Edit 정상 동작

AC-2
Temp/Humidity Edit 정상 동작

AC-3
Validation 정상 동작

AC-4
Dirty 표시 정상 동작

AC-5
Command 기반 Save Flow 유지
```

---

# 17. 다음 단계

다음 DDU:

```text
DDU-TURN-001
Egg Turning Pipeline
```

다음 구현 예정:

- Turning Scheduler
- Turning Policy
- Stepper Control
- Lockdown Stop
- Turning Recovery
