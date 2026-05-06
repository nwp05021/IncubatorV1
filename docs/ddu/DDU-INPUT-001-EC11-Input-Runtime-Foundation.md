# DDU-INPUT-001 EC11 Input Runtime Foundation

## 1. Purpose

본 DDU는 자동 부화기 Premium Industrial UI의 물리 입력 기반을 정의한다.

목표는 EC11 로터리 엔코더를 단순히 읽는 것이 아니라, Runtime Architecture에 맞는 입력 파이프라인을 구성하는 것이다.

```text
EC11 Hardware
    ↓
IEc11Device
    ↓
InputEvent
    ↓
InputEventQueue
    ↓
InputRuntimeTask
    ↓
CommandQueue
    ↓
AppController
```

## 2. Scope

이번 단계에서 구현하는 범위는 다음과 같다.

- EC11 A/B/Button polling
- Button debounce
- Rotation delta decoding
- InputEvent 생성
- InputEventQueue 저장
- InputRuntimeTask 처리
- CommandQueue로 UI 입력 명령 전달
- Serial diagnostics 출력

이번 단계에서 구현하지 않는 범위는 다음과 같다.

- 실제 Screen 전환 정책
- Focus navigation
- Editable field
- Long press command action
- Repeat acceleration
- UI 직접 호출

## 3. Architecture Rules

### 3.1 RuntimeState 직접 수정 금지

Input Runtime은 RuntimeState를 직접 수정하지 않는다.

### 3.2 UI 직접 접근 금지

Input Runtime은 ScreenManager, Widget, Renderer를 직접 호출하지 않는다.

### 3.3 Command 경유

입력은 반드시 CommandQueue에 Command로 전달한다.

### 3.4 Device 책임 제한

EC11 Device는 하드웨어 상태를 읽고 해석 가능한 raw event만 만든다.
Device는 Event 발행, Recovery, RuntimeState 수정, UI 호출을 하지 않는다.

### 3.5 Non-Blocking

입력 처리는 polling 기반이며 delay를 사용하지 않는다.

## 4. Event Model

```cpp
enum class InputEventType
{
    None,
    RotateLeft,
    RotateRight,
    ButtonClick,
    ButtonDown,
    ButtonUp
};
```

## 5. Command Mapping

```text
RotateLeft   -> CommandType::UiRotateLeft
RotateRight  -> CommandType::UiRotateRight
ButtonClick  -> CommandType::UiClick
ButtonDown   -> CommandType::UiButtonDown
ButtonUp     -> CommandType::UiButtonUp
```

이번 DDU에서는 AppController가 해당 명령을 소비하지 않아도 된다.
다음 DDU에서 Screen Navigation 또는 Focus Navigation으로 연결한다.

## 6. Timing Policy

권장 RuntimeScheduler 등록값:

```cpp
TaskId: Input
Priority: Normal
Interval: 5ms
OverrunLimit: 1000us
```

## 7. Hardware Policy

EC11 버튼은 내부 Pull-Up을 사용하고 Active Low로 간주한다.

기본 핀은 다음과 같다.

```cpp
A:   GPIO5
B:   GPIO6
BTN: GPIO4
```

실제 보드 핀 배치가 다르면 `Ec11InputDevice` 생성자 인자로 변경한다.

## 8. Verification

1. 빌드 통과
2. 업로드 후 Serial Monitor 확인
3. EC11 좌회전 시 `RotateLeft` 출력
4. EC11 우회전 시 `RotateRight` 출력
5. 버튼 클릭 시 `ButtonClick` 출력
6. 화면 출력이 깨지지 않아야 함
7. delay 없이 vTaskDelay(1) 기반 유지

## 9. Next DDU

다음 단계는 `DDU-INPUT-002 Screen Navigation Binding`이다.

입력 명령을 ScreenManager에 직접 연결하지 않고, AppController 또는 NavigationController를 통해 정책 기반으로 연결한다.
