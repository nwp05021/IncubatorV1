
# DDU-UI-004 — Progress Screen + Trend UI

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + ST7789
> Dependency:
> - RuntimeState
> - Scheduler
> - Home UI
>
> Estimated Time: 20~40 min

---

# 1. 목적

부화 진행 상태 및 Trend 기반 Ambient UI 구현.

핵심 흐름:

```text
RuntimeState
    ↓
ProgressUiModel
    ↓
TrendBuilder
    ↓
ProgressRenderer
```

목표:

- Day Progress
- ProgressBar
- Mini Trend Graph
- Ambient UX
- Dirty Render

---

# 2. 생성 파일

```text
product/ui/model/ProgressUiModel.h

product/ui/trend/TrendPoint.h
product/ui/trend/TrendBuffer.h

product/ui/viewmodel/ProgressUiModelBuilder.h
product/ui/viewmodel/ProgressUiModelBuilder.cpp

product/ui/render/ProgressRenderer.h
product/ui/render/ProgressRenderer.cpp
```

---

# 3. 핵심 철학

```text
Trend는 살아있는 장비 느낌을 제공해야 한다.
```

---

## 중요한 점

```text
과하면 싸구려가 된다.
```

---

# 4. ProgressUiModel

## 목적

Progress Screen 전용 ViewModel.

---

## ProgressUiModel.h

```cpp
#pragma once

#include <stdint.h>

namespace incubator::ui
{
    struct ProgressUiModel
    {
        uint16_t currentDay = 0;

        uint16_t totalDays = 21;

        bool lockdown = false;

        float currentTempC = 0.0f;

        float currentHumidityPct = 0.0f;

        uint8_t progressPct = 0;
    };
}
```

---

# 5. TrendPoint

## 역할

Trend 데이터 1개.

---

## TrendPoint.h

```cpp
#pragma once

#include <stdint.h>

namespace incubator::ui
{
    struct TrendPoint
    {
        float value = 0.0f;

        uint32_t timestampMs = 0;
    };
}
```

---

# 6. TrendBuffer

## 역할

고정 크기 Trend Buffer.

---

## 핵심 원칙

```text
고정 크기
동적 증가 금지
```

---

## TrendBuffer.h

```cpp
#pragma once

#include "TrendPoint.h"

namespace incubator::ui
{
    class TrendBuffer
    {
    public:
        static constexpr uint16_t Capacity = 64;

    public:
        void push(
            const TrendPoint& point);

        uint16_t count() const;

        const TrendPoint& at(
            uint16_t index) const;

    private:
        TrendPoint m_points[Capacity];

        uint16_t m_head = 0;

        uint16_t m_count = 0;
    };
}
```

---

# 7. ProgressUiModelBuilder

## 역할

```text
RuntimeState
    ↓
ProgressUiModel
```

---

## ProgressUiModelBuilder.h

```cpp
#pragma once

#include "../../domain/RuntimeState.h"
#include "../model/ProgressUiModel.h"

namespace incubator::ui
{
    class ProgressUiModelBuilder
    {
    public:
        void build(
            const incubator::domain::RuntimeState& runtime,
            ProgressUiModel& model);
    };
}
```

---

## ProgressUiModelBuilder.cpp

```cpp
#include "ProgressUiModelBuilder.h"

namespace incubator::ui
{
    using namespace incubator::domain;

    void ProgressUiModelBuilder::build(
        const RuntimeState& runtime,
        ProgressUiModel& model)
    {
        model.currentDay =
            runtime.currentDay;

        model.totalDays =
            runtime.totalDays;

        model.lockdown =
            runtime.lockdown;

        model.currentTempC =
            runtime.currentTempC;

        model.currentHumidityPct =
            runtime.currentHumidityPct;

        if (runtime.totalDays > 0)
        {
            model.progressPct =
                static_cast<uint8_t>(
                    (runtime.currentDay * 100) /
                    runtime.totalDays);
        }
    }
}
```

---

# 8. ProgressRenderer

## 역할

```text
Draw Only
```

---

## 금지

```text
❌ RuntimeState 직접 접근

❌ Alarm 판단

❌ GPIO 접근
```

---

## ProgressRenderer.h

```cpp
#pragma once

#include "../model/ProgressUiModel.h"
#include "../trend/TrendBuffer.h"

namespace incubator::ui
{
    class ProgressRenderer
    {
    public:
        void render(
            const ProgressUiModel& model,
            const TrendBuffer& tempTrend,
            const TrendBuffer& humidityTrend);

    private:
        void renderProgressBar(
            const ProgressUiModel& model);

        void renderTrendGraph(
            const TrendBuffer& trend);
    };
}
```

---

# 9. ProgressBar UX

## 표시 목표

```text
현재 진행 상황 즉시 이해
```

---

## 예시

```text
DAY 07 / 21
[██████------]
```

---

# 10. Trend Graph 정책

## 허용

```text
Soft Line
Tiny Graph
Mini Trend
```

---

## 금지

```text
❌ 실시간 고속 애니메이션

❌ RGB Gradient

❌ 두꺼운 그래프
```

---

# 11. Ambient UX

## 목표

```text
고급 장비 느낌
```

---

## 권장

```text
Soft Refresh
Gentle Highlight
Small Motion
```

---

## 핵심

```text
거의 느껴지지 않을 정도
```

---

# 12. Dirty Render 전략

## 핵심 철학

```text
안 그리는 것이 더 중요하다.
```

---

## 전략

```text
Trend 변경 시만 redraw
```

---

# 13. Main UI Flow

```text
RuntimeState
    ↓
UiModelBuilder
    ↓
ProgressUiModel
    ↓
Renderer
```

---

# 14. Lockdown 표시

## 표시 예시

```text
LOCKDOWN ACTIVE
```

---

## 목적

사용자 즉시 인지.

---

# 15. 핵심 장점

## 1) Trend 구조 분리

Renderer와 Trend 관리 분리.

---

## 2) 고정 메모리 구조

Heap 안정성 향상.

---

## 3) Premium Ambient UX

살아있는 장비 느낌 제공.

---

# 16. 금지 사항

```text
❌ Dynamic Vector growth

❌ Runtime malloc 반복

❌ Full redraw 반복

❌ Renderer 내부 상태 계산
```

---

# 17. Acceptance Criteria

```text
AC-1
ProgressBar 정상 표시

AC-2
TrendBuffer 정상 저장

AC-3
Mini Graph 정상 표시

AC-4
Dirty Render 정상 동작

AC-5
Lockdown 표시 정상 동작
```

---

# 18. 다음 단계

다음 DDU:

```text
DDU-UI-005
Manual Control + Safety UX
```

다음 구현 예정:

- Manual Screen
- Long Hold Confirm
- Heater Manual Control
- Fan Manual PWM
- Dangerous Action UX
