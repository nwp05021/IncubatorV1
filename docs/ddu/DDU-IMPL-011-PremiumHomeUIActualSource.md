
# DDU-IMPL-011 — Premium Home UI Actual Source

## 핵심 흐름

RuntimeState
    ↓
HomeUiModelBuilder
    ↓
HomeUiModel
    ↓
HomeRenderer

## 목표

- Premium Industrial UI
- Dirty Render
- StatusBar
- Overlay Priority
- Large Numeric UX

## 생성 파일

product/ui/model/HomeUiModel.h

product/ui/viewmodel/HomeUiModelBuilder.h
product/ui/viewmodel/HomeUiModelBuilder.cpp

product/ui/render/HomeRenderer.h
product/ui/render/HomeRenderer.cpp

## 핵심 철학

사용자는 1초 안에
현재 상태를 이해할 수 있어야 한다.

## 표시 항목

- Current Temp
- Current Humidity
- Day Progress
- WiFi
- AWS
- SafeMode

## Overlay 우선순위

SafeMode
    > Alarm
        > Dialog
            > Home UI

## Dirty Render 전략

값 변경 시만 redraw

## Acceptance Criteria

- Home UI 표시 정상
- StatusBar 정상
- Overlay 우선순위 정상
- Full redraw 최소화
- UI 응답성 유지
