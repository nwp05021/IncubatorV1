
# DDU-DIAG-001 — Diagnostics + Service Screen

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + ST7789 + EC11
> Dependency:
> - RuntimeState
> - CloudState
> - RecoveryState
>
> Estimated Time: 20~40 min

---

# 1. 목적

진단 및 Factory Service UX 구현.

핵심 흐름:

```text
RuntimeState
    ↓
DiagnosticsService
    ↓
DiagnosticUiModel
    ↓
Service Screen
```

목표:

- Sensor Diagnostics
- GPIO Diagnostics
- WiFi Diagnostics
- Factory Service Mode
- Debug Overlay

---

# 2. 생성 파일

```text
product/diagnostics/DiagnosticsService.h
product/diagnostics/DiagnosticsService.cpp

product/ui/model/DiagnosticUiModel.h

product/ui/viewmodel/DiagnosticUiModelBuilder.h
product/ui/viewmodel/DiagnosticUiModelBuilder.cpp

product/ui/render/DiagnosticRenderer.h
product/ui/render/DiagnosticRenderer.cpp
```

---

# 3. 핵심 철학

```text
진단 화면은
문제를 즉시 찾을 수 있어야 한다.
```

---

# 4. DiagnosticUiModel

## 목적

진단 화면 전용 ViewModel.

---

## DiagnosticUiModel.h

```cpp
#pragma once

namespace incubator::ui
{
    struct DiagnosticUiModel
    {
        bool sensorHealthy = false;

        bool storageHealthy = false;

        bool wifiConnected = false;

        bool awsConnected = false;

        bool safeMode = false;

        uint32_t uptimeMs = 0;

        uint32_t freeHeap = 0;
    };
}
```

---

# 5. DiagnosticsService

## 역할

```text
Health Check
Diagnostics Data
Factory Mode
```

---

## 금지

```text
❌ UI Draw

❌ GPIO 직접 제어

❌ RuntimeState 직접 변경
```

---

## DiagnosticsService.h

```cpp
#pragma once

#include "../domain/RuntimeState.h"

namespace incubator::diagnostics
{
    class DiagnosticsService
    {
    public:
        DiagnosticsService(
            incubator::domain::RuntimeState& runtime);

    public:
        void tick(uint32_t nowMs);

        uint32_t getFreeHeap() const;

    private:
        incubator::domain::RuntimeState& m_runtime;

        uint32_t m_lastTickMs = 0;

        static constexpr uint32_t TickIntervalMs =
            5000;
    };
}
```

---

# 6. DiagnosticUiModelBuilder

## 역할

```text
RuntimeState
    ↓
DiagnosticUiModel
```

---

## DiagnosticUiModelBuilder.h

```cpp
#pragma once

#include "../../domain/RuntimeState.h"
#include "../../cloud/CloudState.h"

#include "../model/DiagnosticUiModel.h"

namespace incubator::ui
{
    class DiagnosticUiModelBuilder
    {
    public:
        void build(
            const incubator::domain::RuntimeState& runtime,
            const incubator::cloud::CloudState& cloud,
            DiagnosticUiModel& model);
    };
}
```

---

## DiagnosticUiModelBuilder.cpp

```cpp
#include "DiagnosticUiModelBuilder.h"

namespace incubator::ui
{
    using namespace incubator::domain;

    void DiagnosticUiModelBuilder::build(
        const RuntimeState& runtime,
        const incubator::cloud::CloudState& cloud,
        DiagnosticUiModel& model)
    {
        model.sensorHealthy =
            runtime.sensorHealthy;

        model.storageHealthy =
            runtime.storageHealthy;

        model.safeMode =
            runtime.safeMode;

        model.wifiConnected =
            cloud.wifiConnected;

        model.awsConnected =
            cloud.mqttConnected;

        model.uptimeMs =
            runtime.uptimeMs;
    }
}
```

---

# 7. DiagnosticRenderer

## 역할

```text
Draw Only
```

---

## 표시 예시

```text
SENSOR      OK
STORAGE     OK
WIFI        OK
AWS         OK

FREE HEAP
182 KB
```

---

## DiagnosticRenderer.h

```cpp
#pragma once

#include "../model/DiagnosticUiModel.h"

namespace incubator::ui
{
    class DiagnosticRenderer
    {
    public:
        void render(
            const DiagnosticUiModel& model);
    };
}
```

---

# 8. Factory Service Mode

## 목적

생산/수리용 진단.

---

## 기능 예시

```text
Relay Test
Fan Test
Display Test
Sensor Test
WiFi Test
```

---

## 보호 정책

```text
Long Hold Enter
```

권장.

---

# 9. Debug Overlay

## 목적

개발 중 상태 확인.

---

## 표시 예시

```text
FPS
Loop Time
Heap
WiFi RSSI
```

---

## 금지

```text
❌ Release Build 상시 표시
```

---

# 10. Health 정책

## 핵심 목표

```text
현재 시스템 상태 즉시 이해
```

---

## 상태 기준

| 상태 | 의미 |
|---|---|
| OK | 정상 |
| WARN | 주의 |
| FAIL | 오류 |

---

# 11. Dirty Render 전략

## 핵심 철학

```text
변경 시만 redraw
```

---

## 예시

```text
WiFi 상태 변경
    ↓
WiFi 영역 redraw
```

---

# 12. Main UI Flow

```text
RuntimeState
    ↓
DiagnosticsService
    ↓
DiagnosticUiModel
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

    diagnostics.tick(now);

    sensorManager.tick(now);

    scheduler.tick(now);

    climate.tick(now);

    turning.tick(now);

    fan.tick(now);

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

# 14. 핵심 장점

## 1) 진단 구조 독립성

UI/Module 분리 유지.

---

## 2) Factory 대응 가능

생산/수리 모드 확장 가능.

---

## 3) RuntimeState 중심 구조

진단 데이터 일관성 확보.

---

# 15. 금지 사항

```text
❌ Diagnostics 내부 UI Draw

❌ Blocking Test Loop

❌ GPIO direct test without safety

❌ Release Build debug spam
```

---

# 16. Acceptance Criteria

```text
AC-1
Health 상태 정상 표시

AC-2
WiFi/AWS 상태 표시

AC-3
Heap 표시 정상

AC-4
Factory Mode 진입 가능

AC-5
Dirty Render 정상 동작
```

---

# 17. 다음 단계

다음 DDU:

```text
DDU-EVENT-001
Event & Notification Pipeline
```

다음 구현 예정:

- Alarm Event
- Toast Notification
- Event History
- Critical Event Queue
- Cloud Notification
