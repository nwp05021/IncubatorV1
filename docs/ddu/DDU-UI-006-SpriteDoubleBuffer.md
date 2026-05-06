# DDU-UI-006 Sprite Double Buffer

## 1. 목적

Premium Industrial UI 렌더링에서 화면 깜빡임을 줄이고, Widget 렌더링 결과를 하나의 프레임으로 안정적으로 출력한다.

본 DDU는 기존 RuntimeState → UiModel → Diff → DirtyFlags → Widget Renderer → DisplayDevice 흐름을 유지한다.

## 2. 범위

이번 단계의 범위는 다음으로 제한한다.

- LovyanGFX Sprite 기반 Full Frame Buffer 추가
- `St7789DisplayDevice` 내부에 Sprite 구현 은닉
- `IDisplayDevice`에는 선택적 Sprite Frame API만 추가
- `GraphicHomeRenderer`는 Sprite 사용 가능 시 Full Frame Render 수행
- Sprite 할당 실패 시 기존 Dirty Partial Render로 자동 fallback

## 3. 비범위

다음은 이번 DDU에서 구현하지 않는다.

- Widget별 개별 Sprite
- Dirty Region Sprite Patch
- Animation System
- Screen Manager
- Alarm Overlay 고도화
- PSRAM 진단 UI

## 4. 설계 원칙

### 4.1 RuntimeState 중심 유지

Sprite는 표시 방식일 뿐이며 상태를 소유하지 않는다.

```text
RuntimeState
    ↓
HomeUiModel
    ↓
HomeUiDiff
    ↓
DirtyFlags
    ↓
GraphicHomeRenderer
    ↓
IDisplayDevice
    ↓
St7789DisplayDevice Sprite
```

### 4.2 Device 내부 은닉

LovyanGFX Sprite는 `St7789DisplayDevice` 내부 구현 세부사항이다.

Renderer / Widget / Screen은 `LGFX_Sprite`를 직접 알면 안 된다.

### 4.3 Fallback 우선

Sprite Buffer 생성 실패 시 화면 출력은 중단되지 않는다.

```text
Sprite 가능     → Full Frame Sprite Render
Sprite 불가능   → 기존 Dirty Partial Render
```

## 5. Public Contract

`IDisplayDevice`는 다음 선택적 API를 제공한다.

```cpp
virtual bool beginSpriteFrame(uint16_t clearColor);
virtual void endSpriteFrame();
virtual bool isSpriteFrameActive() const;
```

기본 구현은 Sprite 미지원으로 동작한다.

## 6. Memory 정책

ST7789 Landscape 기준 버퍼 크기:

```text
320 × 240 × 2 bytes = 153,600 bytes
```

PSRAM 사용 환경에서는 허용 가능한 수준이다.
단, Sprite 생성 실패 가능성을 항상 고려해야 한다.

## 7. Render 정책

### Sprite 사용 가능

DirtyFlags는 Render Trigger로만 사용한다.
실제 출력은 전체 프레임을 Sprite에 그린 뒤 한 번에 push한다.

### Sprite 사용 불가

기존과 동일하게 DirtyFlags 기준 Partial Render를 수행한다.

## 8. 실기기 검증 항목

- 부팅 후 첫 화면이 정상 출력되는가
- 1초 단위 온도/습도 변경 시 깜빡임이 줄어드는가
- 출력 도중 화면 깨짐이 없는가
- Sprite 생성 실패 시에도 기존 방식으로 출력되는가
- `vTaskDelay(1)` 기반 Runtime Yield가 유지되는가

## 9. 다음 DDU

다음 단계는 `DDU-UI-007 Widget System 고도화`로 진행한다.

- WidgetBase
- WidgetContext
- WidgetStyle
- CardWidget 공통화
- Icon/Text/Value 단위 컴포넌트 분리
