
# DDU-UI-001 — Premium Home UI Foundation

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + ST7789
> Dependency:
> - RuntimeState
> - AlarmState
> - Scheduler
>
> Estimated Time: 20~40 min

---

# 1. 목적

Premium Industrial Home UI 기반 구축.

핵심 흐름:

```text
RuntimeState
    ↓
UiViewModelBuilder
    ↓
UiModel
    ↓
Renderer
```

목표:

- 현재 상태 1초 이해
- 큰 숫자 중심
- Dirty Render
- StatusBar
- Overlay 기반

---

# 2. 생성 파일

```text
product/ui/model/HomeUiModel.h

product/ui/viewmodel/HomeUiModelBuilder.h
product/ui/viewmodel/HomeUiModelBuilder.cpp

product/ui/render/HomeRenderer.h
product/ui/render/HomeRenderer.cpp

product/ui/components/StatusBar.h
product/ui/components/StatusBar.cpp
```

---

# 3. 핵심 철학

```text
Premium UI는 화려함이 아니라 신뢰감이다.
```

---

# 4. UI 구조

```text
┌──────────────────────────────┐
│ StatusBar                    │
├──────────────────────────────┤
│                              │
│ TemperatureCard              │
│ HumidityCard                 │
│                              │
├──────────────────────────────┤
│ ProgressBar                  │
└──────────────────────────────┘
```

---

# 5. HomeUiModel

## 목적

Renderer 전용 ViewModel.

---

## HomeUiModel.h

```cpp
#pragma once

#include <stdint.h>

namespace incubator::ui
{
    struct HomeUiModel
    {
        float currentTempC = 0.0f;

        float targetTempC = 0.0f;

        float currentHumidityPct = 0.0f;

        float targetHumidityPct = 0.0f;

        uint16_t currentDay = 0;

        uint16_t totalDays = 21;

        bool wifiConnected = false;

        bool awsConnected = false;

        bool safeMode = false;

        bool highTempAlarm = false;

        bool lowTempAlarm = false;
    };
}
```

---

# 6. UiViewModelBuilder

## 역할

```text
RuntimeState
    ↓
UiModel 변환
```

---

## 금지

```text
❌ UI 직접 RuntimeState 접근

❌ Renderer 내부 상태 계산
```

---

## HomeUiModelBuilder.h

```cpp
#pragma once

#include "../../domain/RuntimeState.h"
#include "../model/HomeUiModel.h"

namespace incubator::ui
{
    class HomeUiModelBuilder
    {
    public:
        void build(
            const incubator::domain::RuntimeState& runtime,
            HomeUiModel& model);
    };
}
```

---

## HomeUiModelBuilder.cpp

```cpp
#include "HomeUiModelBuilder.h"

namespace incubator::ui
{
    using namespace incubator::domain;

    void HomeUiModelBuilder::build(
        const RuntimeState& runtime,
        HomeUiModel& model)
    {
        model.currentTempC =
            runtime.currentTempC;

        model.targetTempC =
            runtime.targetTempC;

        model.currentHumidityPct =
            runtime.currentHumidityPct;

        model.targetHumidityPct =
            runtime.targetHumidityPct;

        model.currentDay =
            runtime.currentDay;

        model.totalDays =
            runtime.totalDays;

        model.wifiConnected =
            runtime.wifiConnected;

        model.awsConnected =
            runtime.awsConnected;

        model.safeMode =
            runtime.safeMode;

        model.highTempAlarm =
            runtime.highTempAlarm;

        model.lowTempAlarm =
            runtime.lowTempAlarm;
    }
}
```

---

# 7. HomeRenderer

## 역할

```text
Draw Only
```

---

## 금지

```text
❌ GPIO 접근

❌ 상태 변경

❌ Alarm 계산
```

---

## HomeRenderer.h

```cpp
#pragma once

#include "../model/HomeUiModel.h"

namespace incubator::ui
{
    class HomeRenderer
    {
    public:
        void render(
            const HomeUiModel& model);

    private:
        void renderTemperature(
            const HomeUiModel& model);

        void renderHumidity(
            const HomeUiModel& model);

        void renderProgress(
            const HomeUiModel& model);
    };
}
```

---

# 8. StatusBar

## 역할

현재 시스템 상태 표시.

---

## 표시 항목

```text
Time
Day
WiFi
AWS
Alarm
```

---

## StatusBar.h

```cpp
#pragma once

#include "../model/HomeUiModel.h"

namespace incubator::ui
{
    class StatusBar
    {
    public:
        void render(
            const HomeUiModel& model);
    };
}
```

---

# 9. Dirty Render 전략

## 핵심 철학

```text
안 그리는 것이 더 중요하다.
```

---

## 전략

```text
값 변경 시만 redraw
```

---

## 권장

```text
Dirty Region Render
```

---

# 10. 숫자 디자인 규칙

## 핵심 목표

```text
1초 안에 상태 이해
```

---

## 규칙

```text
현재 온도:
가장 크게

목표 온도:
작게

Trend:
아주 작게
```

---

# 11. Alarm Overlay 연계

## SafeMode

```text
최상위 Overlay
```

---

## 우선순위

```text
SafeMode
    > Alarm
        > Dialog
            > Toast
```

---

# 12. Main UI Flow

```text
RuntimeState
    ↓
UiModelBuilder
    ↓
UiModel
    ↓
Renderer
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

    cloud.tick(now);
}
```

---

# 14. 핵심 장점

## 1) UI 독립성

```text
UI는 RuntimeState 읽기 전용
```

---

## 2) Renderer 단순화

Renderer는 Draw만 수행.

---

## 3) Premium UI 유지 용이

Component 단위 확장 가능.

---

# 15. 금지 사항

```text
❌ Renderer 내부 상태 변경

❌ UI direct GPIO control

❌ UI direct RuntimeState mutation

❌ Full redraw 남발
```

---

# 16. Acceptance Criteria

```text
AC-1
온도 카드 정상 표시

AC-2
습도 카드 정상 표시

AC-3
StatusBar 표시

AC-4
Dirty Render 구조 유지

AC-5
SafeMode Overlay 우선 표시
```

---

# 17. 다음 단계

다음 DDU:

```text
DDU-UI-002
EC11 Navigation & Focus System
```

다음 구현 예정:

- UiNavigator
- Focus System
- Encoder Input
- Menu Navigation
- Dialog Navigation
- Long Hold Confirm
