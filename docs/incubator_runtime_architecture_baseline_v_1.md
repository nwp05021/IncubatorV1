# Incubator Runtime Architecture Baseline v1

## 목적

본 문서는 ESP32-S3 기반 Premium Incubator Runtime의 현재 아키텍처 기준점(Baseline)을 고정하기 위한 문서이다.

현재 구조는 다음 핵심 철학을 기반으로 한다.

- Data-Centric Firmware Architecture
- Single Source of Truth
- Command-Driven State Mutation
- Offline First
- SafeMode 우선
- Premium Industrial UI
- 사람이 완전히 이해 가능한 구조

---

# 1. 전체 아키텍처

```text
ESP-IDF
    ↓
FreeRTOS
    ↓
Arduino as Component
    ↓
Runtime Layer
    ↓
Product Layer
```

---

# 2. Product Layer 구조

```text
product/
├── app/
├── cloud/
├── diagnostics/
├── devices/
├── domain/
├── event/
├── logging/
├── modules/
├── performance/
├── runtime/
├── time/
└── ui/
```

---

# 3. RuntimeState 중심 구조

## 핵심 원칙

모든 Runtime 상태는 RuntimeState를 중심으로 흐른다.

```text
Sensor
    ↓
RuntimeState
    ↓
UI / Cloud / Alarm / Telemetry
```

## 목표

- Single Source of Truth
- 상태 추적 가능성 확보
- 디버깅 단순화
- Event Replay 가능성 확보
- Cloud Shadow 대응

---

# 4. Runtime Scheduler

## 구조

```text
RuntimeScheduler
    ↓
RuntimeTask
    ↓
TickHandler
```

## RuntimePriority

```cpp
enum class RuntimePriority
{
    Critical,
    Normal,
    Low
};
```

---

# 5. Scheduler 정책

## Critical

- Sensor
- Climate
- Recovery
- Performance

## Normal

- UI
- Event
- Controller

## Low

- Diagnostics
- Serial Logger
- Debug Overlay

---

# 6. Runtime Timing 정책

## 기본 원칙

모든 Runtime은 Non-Blocking 구조를 유지한다.

## 금지

```cpp
// 금지
 delay(1000);
```

## 허용

```cpp
vTaskDelay(1);
```

위 호출은 FreeRTOS Idle Task 실행 보장을 위한 Yield 목적이다.

---

# 7. Overrun Detection

각 RuntimeTask는 다음 정보를 가진다.

```cpp
lastExecTimeUs
maxExecTimeUs
overrunLimitUs
overrunCount
```

## 목표

- Runtime spike 감지
- UI 성능 문제 분석
- Cloud blocking 분석
- Sensor starvation 방지

---

# 8. Performance Monitor

## 수집 항목

- Loop Count
- Last Loop Time
- Max Loop Time
- Free Heap
- Min Free Heap
- Loop Overrun

## 목표

Runtime Determinism 확보.

---

# 9. Logging Architecture

## 구조

```text
SerialLogger
    ↓
Ring Buffer
    ↓
Low Priority Flush
```

## 목적

- Serial blocking 제거
- Scheduler timing 오염 방지
- Runtime stability 확보

---

# 10. UI Architecture

## 전체 흐름

```text
RuntimeState
    ↓
HomeUiModelBuilder
    ↓
HomeUiModel
    ↓
HomeUiDiff
    ↓
DirtyFlags
    ↓
Renderer
    ↓
DisplayDevice
```

---

# 11. Dirty Render 구조

## 목적

전체 redraw를 방지하고 변경 영역만 갱신.

## 구조

```text
Previous Snapshot
    ↓
Diff Compare
    ↓
DirtyFlags
    ↓
Partial Render
```

---

# 12. Widget Architecture

## 목적

Renderer 비대화 방지.

## 구조

```text
widgets/
    StatusBarWidget
    TemperatureCardWidget
    ProgressWidget
```

---

# 13. Layout Architecture

## 목적

좌표 하드코딩 제거.

## 구조

```cpp
struct HomeLayout
{
    Rect statusBar;
    Rect tempCard;
    Rect humidityCard;
    Rect progress;
};
```

---

# 14. Display Architecture

## 구조

```text
GraphicHomeRenderer
    ↓
IDisplayDevice
    ↓
St7789DisplayDevice
```

## 목표

Display backend 교체 가능성 확보.

---

# 15. UiTheme

## 목적

색상 및 UI 정책 중앙화.

## 포함 항목

- Color
- Surface
- Accent
- Warning
- Danger
- Text

---

# 16. Device Layer 정책

## Device 역할

- 하드웨어 접근만 수행
- Event 발행 금지
- Recovery 금지
- 상태 판단 금지

---

# 17. Module Layer 정책

## Module 역할

- RuntimeState 기반 제어
- Device orchestration
- Policy 수행
- Non-Blocking Tick 수행

---

# 18. SafeMode 정책

## 핵심 원칙

불확실하면 출력을 차단한다.

## SafeMode 발생 조건 예시

- Sensor invalid
- Storage invalid
- Runtime fault
- Overheat
- Recovery failure

---

# 19. Cloud Architecture

## 현재 구조

```text
WifiManager
    ↓
AwsIotClient
    ↓
Telemetry
```

## 향후 확장 예정

- Shadow Sync
- Offline Queue
- Retry Policy
- Persistent Telemetry

---

# 20. 향후 구현 예정 우선순위

## Runtime

- Tick Budget System
- Core Affinity
- FreeRTOS Task 분리
- EventBus 실제화

## UI

- Sprite Double Buffer
- Animation System
- Screen Manager
- EC11 Input System
- Alarm Overlay

## Storage

- Atomic Save
- Versioning
- Validation
- Rollback

## Cloud

- MQTT Queue
- Shadow Sync
- Offline Replay
- OTA

---

# 21. Coding Contract

## Namespace

```cpp
incubator::runtime
incubator::ui
incubator::devices
```

## 변수명

```cpp
runtimeDiagnostics
runtimeScheduler
performanceMonitor
```

## 금지

```cpp
ui ui;
diagnostics diagnostics;
```

---

# 22. Include Contract

CPP는 자신이 사용하는 모든 타입의 header를 직접 include 해야 한다.

## 금지

```cpp
간접 include 의존
```

---

# 23. 현재 프로젝트 상태 평가

현재 구조는:

```text
Industrial Embedded Runtime 초기 구조
```

수준으로 판단된다.

특히 다음이 매우 중요하다.

```text
RuntimeState 중심 구조
```

가 끝까지 유지되고 있다는 점이다.

이는 이후:

- OTA
- Recovery
- Cloud Shadow
- Event Replay
- Telemetry
- Alarm History

까지 모두 확장 가능하게 만든다.

