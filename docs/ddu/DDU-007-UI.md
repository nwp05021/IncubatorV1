# DDU-007 — 로컬 UI (UiController + MainUiRenderer 5-Page)
> **Document ID**: DDU-007  
> **Version**: 1.0  
> **상위 문서**: INC-IMPL-001 §12  
> **의존 DDU**: DDU-001, DDU-003, DDU-006  
> **Codex 작업 시간 예상**: 30~40분

---

## 작업 목표

이 DDU 완료 후:
- EC11 회전으로 5개 페이지를 이동한다
- LongClick으로 항상 Page 0(Main)으로 돌아온다
- Page 0: 온도/습도 대형 표시 + 목표값 + 4개 상태 아이콘
- Page 1: 부화 진행률 + 전란 타이머
- Page 2: 수동 제어 (Heater/Humidifier/Turner/Fan)
- Page 3: Day Plan 편집 → AppController::PatchPlanRow
- Page 4: 시스템 정보
- SafeMode 시 오버레이 표시

---

## 1. 생성할 파일 목록

```
include/ui/UiLayout.h           ← 좌표 상수 (헤더 전용)
include/ui/UiColors.h           ← 색상 상수 (헤더 전용)
include/ui/UiModel.h            ← UI 데이터 구조체
include/ui/UiController.h
src/ui/UiController.cpp
include/ui/MainUiRenderer.h
src/ui/MainUiRenderer.cpp
```

---

## 2. UiLayout.h

```cpp
// include/ui/UiLayout.h
#pragma once

namespace incubator::ui::Layout
{
    static constexpr int kScreenW     = 240;
    static constexpr int kScreenH     = 320;

    // Header
    static constexpr int kHeaderY     = 0;
    static constexpr int kHeaderH     = 30;
    static constexpr int kHeaderTextY = 10;
    static constexpr int kHeaderLineY = 29;

    // Body
    static constexpr int kBodyY       = 30;
    static constexpr int kBodyH       = 170;
    static constexpr int kBodyEndY    = 199;

    // Footer
    static constexpr int kFooterY     = 200;
    static constexpr int kFooterH     = 40;
    static constexpr int kFooterTextY = 220;
    static constexpr int kFooterLineY = 199;

    // Footer 아이콘 x 좌표
    static constexpr int kIconHtrX    = 5;
    static constexpr int kIconHumX    = 58;
    static constexpr int kIconTrnX    = 111;
    static constexpr int kIconFanX    = 164;
}
```

---

## 3. UiColors.h

```cpp
// include/ui/UiColors.h
// LovyanGFX RGB565 기준
#pragma once
#include <cstdint>

namespace incubator::ui::Color
{
    static constexpr uint32_t kBg         = 0x0000U;  // Black
    static constexpr uint32_t kHeader     = 0x1082U;  // Dark gray
    static constexpr uint32_t kFooter     = 0x1082U;
    static constexpr uint32_t kText       = 0xFFFFU;  // White
    static constexpr uint32_t kTextDim    = 0x7BEFU;  // Gray
    static constexpr uint32_t kAccentTemp = 0xFD20U;  // Orange
    static constexpr uint32_t kAccentHumi = 0x07FFU;  // Cyan
    static constexpr uint32_t kAlarmHigh  = 0xF800U;  // Red
    static constexpr uint32_t kAlarmLow   = 0x001FU;  // Blue
    static constexpr uint32_t kOnIcon     = 0x07E0U;  // Green
    static constexpr uint32_t kOffIcon    = 0x4208U;  // Dark gray
    static constexpr uint32_t kLockdown   = 0xFFE0U;  // Yellow
    static constexpr uint32_t kSelected   = 0x3166U;  // Highlight blue-gray
    static constexpr uint32_t kDivider    = 0x4208U;
    static constexpr uint32_t kProgress   = 0x07E0U;  // Green (진행 바)
}
```

---

## 4. UiModel.h

```cpp
// include/ui/UiModel.h
// ★ MainUiRenderer는 이 구조체만 읽는다.
//   RuntimeState 직접 접근 절대 금지.
#pragma once
#include <cstdint>

namespace incubator::ui
{
    enum class EditField : uint8_t { None, Day, Temp, Humidity, Turning };

    struct UiModel
    {
        // ── Page 0/1 공통 ─────────────────────────────────────────
        float    displayTempC      = 0.0f;
        float    displayHumidPct   = 0.0f;
        float    targetTempC       = 37.5f;
        float    targetHumidPct    = 55.0f;
        bool     heaterOn          = false;
        bool     humidifierOn      = false;
        bool     turnerOn          = false;
        bool     fanOn             = false;
        bool     tempAlarm         = false;
        bool     humiAlarm         = false;
        bool     tempSensorFault   = false;
        bool     humiSensorFault   = false;
        uint16_t currentDay        = 0;
        uint16_t totalDays         = 21;
        uint8_t  progressPct       = 0;
        bool     batchActive       = false;
        bool     lockdownActive    = false;
        bool     turningEnabled    = true;
        uint16_t nextTurningInMin  = 0;
        uint16_t lockdownStartDay  = 19;

        // ── Page 3: Plan Edit ─────────────────────────────────────
        uint16_t  editDay          = 1;
        float     editTempC        = 37.5f;
        float     editHumidPct     = 55.0f;
        bool      editTurning      = true;
        uint16_t  editIntervalMin  = 120;
        bool      editOverridden   = false;
        EditField activeEditField  = EditField::None;

        // ── Page 4: System ────────────────────────────────────────
        uint32_t bootCount         = 0;
        uint32_t uptimeMs          = 0;
        bool     cloudConnected    = false;
        char     ipAddress[16]     = {};
        char     batchId[16]       = {};
        char     fwVersion[12]     = "v1.0.0";

        // ── UI 네비게이션 ─────────────────────────────────────────
        uint8_t  activePage        = 0;    // 0~4
        bool     safeMode          = false;
        int8_t   manualCursor      = 0;    // Page 2: 0=Heater,1=Humid,2=Turner,3=Fan
    };
}
```

---

## 5. UiController

### 5.1 헤더

```cpp
// include/ui/UiController.h
#pragma once
#include "UiModel.h"
#include "domain/RuntimeState.h"
#include "domain/IncubationPlanTable.h"
#include "app/AppController.h"
#include "devices/Ec11Encoder.h"
#include <cstdint>

namespace incubator::ui
{
    class UiController
    {
    public:
        static constexpr uint32_t kSyncIntervalMs = 200U;

        UiController(UiModel&                           model,
                     const domain::RuntimeState&        state,
                     const domain::IncubationPlanTable& plan,
                     app::AppController&                ctrl,
                     devices::Ec11Encoder&              encoder)
            : m_model(model), m_state(state), m_plan(plan),
              m_ctrl(ctrl), m_encoder(encoder) {}

        void tick(uint32_t nowMs);

    private:
        UiModel&                           m_model;
        const domain::RuntimeState&        m_state;
        const domain::IncubationPlanTable& m_plan;
        app::AppController&                m_ctrl;
        devices::Ec11Encoder&              m_encoder;

        uint32_t m_lastSyncMs = 0;

        // 상태 동기화
        void syncFromState();

        // 입력 처리
        void handleInput();
        void handleDelta(int delta);
        void handleClick();
        void handleLongPress();

        // 페이지별 입력
        void page0Delta(int d);
        void page1Delta(int d);
        void page2Delta(int d);
        void page2Click();
        void page3Delta(int d);
        void page3Click();
        void page3LongPress();

        // Page 3 편집 저장
        void savePlanEdit();

        // Page 3: editDay의 현재 Plan Row 로드
        void loadEditRow(uint16_t day);
    };
}
```

### 5.2 구현 핵심 로직

```
// syncFromState() — RuntimeState → UiModel 복사 (200ms 주기)
m_model.displayTempC    = m_state.currentTempC
m_model.displayHumidPct = m_state.currentHumidityPct
m_model.targetTempC     = m_state.targetTempC
m_model.targetHumidPct  = m_state.targetHumidityPct
m_model.heaterOn        = m_state.heaterOn
m_model.humidifierOn    = m_state.humidifierOn
m_model.turnerOn        = m_state.turnerOn
m_model.tempAlarm       = m_state.tempAlarmActive
m_model.humiAlarm       = m_state.humiAlarmActive
m_model.tempSensorFault = !m_state.tempSensorOk
m_model.humiSensorFault = !m_state.humiSensorOk
m_model.currentDay      = m_state.currentDay
m_model.totalDays       = m_state.totalDays
m_model.progressPct     = DayResolver::progressPct(m_state.currentDay, m_state.totalDays)
m_model.batchActive     = m_state.batchActive
m_model.lockdownActive  = m_state.lockdownActive
m_model.turningEnabled  = m_state.turningEnabled
m_model.nextTurningInMin= m_state.nextTurningInMin
m_model.safeMode        = m_state.safeMode
m_model.bootCount       = m_state.bootCount
m_model.uptimeMs        = m_state.uptimeMs
m_model.cloudConnected  = m_state.cloudConnected
memcpy(m_model.ipAddress, m_state.ipAddress, 16)

// handleDelta():
// Page 0: 페이지 이동 (delta>0: +1, delta<0: -1, 0~4 wrap)
// Page 2: manualCursor 이동 (0~3 클램핑)
// Page 3:
//   editField==None: editDay ±1 (1~totalDays 클램핑)
//   editField==Temp: editTempC ±0.1°C (30.0~42.0 클램핑)
//   editField==Humidity: editHumidPct ±1.0% (20.0~90.0 클램핑)
//   editField==Turning: toggle turningEnabled

// handleClick():
// Page 2: manualCursor 항목 토글 (AppController::applyCommand 호출)
// Page 3:
//   editField==None: editField = Temp
//   editField==Temp: editField = Humidity
//   editField==Humidity: editField = Turning
//   editField==Turning: editField = None

// handleLongPress():
// 모든 페이지: activePage = 0 (홈 복귀)
// Page 3: savePlanEdit() 후 activePage = 0

// page2Click():
// cursor=0: AppController::applyCommand(heaterOn ? Cmd::HeaterOff : Cmd::HeaterOn)
// cursor=1: AppController::applyCommand(humidOn  ? Cmd::HumidOff  : Cmd::HumidOn)
// cursor=2: AppController::applyCommand(turnerOn ? Cmd::TurnerOff : Cmd::TurnerOn)
// cursor=3: (Fan duty toggle 50/0)

// savePlanEdit():
// row.day              = m_model.editDay
// row.targetTempC      = m_model.editTempC
// row.targetHumidityPct= m_model.editHumidPct
// row.turningEnabled   = m_model.editTurning
// row.turningIntervalMin = m_model.editIntervalMin
// AppController::applyCommand(Cmd::PatchPlanRow, &row, sizeof(row))
// m_model.activeEditField = EditField::None
```

---

## 6. MainUiRenderer

### 6.1 헤더

```cpp
// include/ui/MainUiRenderer.h
// ★ UiModel만 읽는다. RuntimeState, AppController 직접 접근 금지.
#pragma once
#include "UiModel.h"
#include "devices/St7789Display.h"
#include <cstdint>

namespace incubator::ui
{
    class MainUiRenderer
    {
    public:
        static constexpr uint32_t kRenderIntervalMs = 100U;

        MainUiRenderer(UiModel&                model,
                       devices::St7789Display& display)
            : m_model(model), m_display(display) {}

        void render(uint32_t nowMs);

    private:
        UiModel&                m_model;
        devices::St7789Display& m_display;
        uint32_t                m_lastRenderMs = 0;

        void renderHeader();
        void renderFooter();
        void renderSafeMode();
        void renderPage0();   // Main
        void renderPage1();   // Status
        void renderPage2();   // Manual
        void renderPage3();   // Plan Edit
        void renderPage4();   // System

        // 공통 헬퍼
        void drawStatusIcon(int x, int y, const char* label, bool on);
        void drawProgressBar(int x, int y, int w, int h,
                             uint8_t pct, uint32_t color);
        void formatTemp(char* buf, size_t len, float t, bool fault);
        void formatHumi(char* buf, size_t len, float h, bool fault);
        void formatUptime(char* buf, size_t len, uint32_t ms);
        void formatEpochDate(char* buf, size_t len, uint32_t epoch);
    };
}
```

### 6.2 각 페이지 렌더 명세

#### render() 진입 흐름

```cpp
void MainUiRenderer::render(uint32_t nowMs)
{
    if (nowMs - m_lastRenderMs < kRenderIntervalMs) return;
    m_lastRenderMs = nowMs;

    auto& gfx = m_display.gfx();
    gfx.startWrite();
    gfx.fillScreen(Color::kBg);

    if (m_model.safeMode) {
        renderSafeMode();
        renderFooter();
        gfx.endWrite();
        return;
    }

    renderHeader();

    switch (m_model.activePage) {
        case 0: renderPage0(); break;
        case 1: renderPage1(); break;
        case 2: renderPage2(); break;
        case 3: renderPage3(); break;
        case 4: renderPage4(); break;
        default: renderPage0(); break;
    }

    renderFooter();
    gfx.endWrite();
}
```

#### renderHeader()

```
// y=0..29, 배경색 kHeader
// 좌: "●" (kAccentTemp) + 페이지명
// 중: 알람 시 "!" 깜박 (kAlarmHigh)
// 우: "D-07" (kTextDim) + " 08:12" (kText)
// 하단 구분선: y=29, kDivider

페이지명 매핑:
  Page 0: "INCUBATOR"
  Page 1: "STATUS"
  Page 2: "MANUAL"
  Page 3: "PLAN EDIT"
  Page 4: "SYSTEM"
```

#### renderFooter()

```
// y=200..239, 배경색 kFooter
// 상단 구분선: y=199, kDivider
// 좌측 아이콘 (drawStatusIcon):
//   x=5:   "HTR" (heaterOn → kOnIcon, else kOffIcon)
//   x=58:  "HUM" (humidifierOn)
//   x=111: "TRN" (turnerOn)
//   x=164: "FAN" (fanOn)
// 우측: "7일차" (kTextDim) + " [=====    ]" 진행 바
//   진행 바: x=220, y=215, w=90, h=12, pct=progressPct
```

#### renderSafeMode()

```
// Body 전체 빨간 배경
// 중앙: "!! SAFE MODE !!" (size=3, kAlarmHigh)
// 그 아래: "Check sensors / reboot" (size=1, kText)
// 깜박 효과: 500ms 주기 토글 (정적 counter 활용)
```

#### renderPage0() — Main

```
렌더 좌표 (픽셀):

[온도 대형]
  if (tempSensorFault): "---" (size=4, kTextDim)
  else if (tempAlarm): text (size=4, kAlarmHigh)
  else: text (size=4, kAccentTemp)
  위치: x=10, y=70

[습도 대형]
  위치: x=175, y=70
  색상: 온도와 동일 패턴 (kAccentHumi / kAlarmHigh)

[목표 온도 소형]
  "↑37.5C" x=10, y=130, size=1, kTextDim

[목표 습도 소형]
  "↑60%" x=175, y=130, size=1, kTextDim

[도움말]
  "< Rotate:page  LongPress:menu >" x=10, y=165, size=1, kTextDim

[배치 비활성 시]
  중앙 "No Active Batch" (size=2, kTextDim) → 시작 방법 안내
```

#### renderPage1() — Status

```
y=40:  "Progress: [=========  ] 33%"  progressBar + text
y=65:  "Today:  37.5°C   60%"         targetTempC / targetHumidPct
y=85:  "Turning: every 120 min"
y=100: "Next turn: 45 min"
y=115: "Lockdown: Day 19~"  (lockdownActive 시 kLockdown 색상)
y=135: "Remaining: 14 days"
y=150: "Finish: YYYY-MM-DD"  (completionEpoch 기반)
```

#### renderPage2() — Manual

```
4개 항목 목록 (y=45 시작, 간격 32px):
  항목: "  HEATER   [ON ]"
        "  HUMID    [OFF]"
        "  TURNER   [OFF]"
        "  FAN      [ 60%]"

선택된 항목(manualCursor): 배경 kSelected 강조 (fillRect)
ON/OFF 뱃지: ON→kOnIcon, OFF→kOffIcon

하단: "Click:toggle  LongPress:home"
```

#### renderPage3() — Plan Edit

```
y=38: "Day: [ 07 ]"  ← editDay, 선택 시 강조

3개 편집 항목 (y=60 시작, 간격 34px):
  activeEditField에 따라 해당 항목 강조 (kSelected 배경)

  "> Temp:     37.5 C"   ← editTempC
  "  Humidity: 60.0 %"   ← editHumidPct
  "  Turning:  ON / 120min" ← editTurning / editIntervalMin

편집 모드 힌트:
  editField==None: "Rotate:day  Click:edit  LongPress:save"
  editField!=None: "Rotate:change  Click:next  LongPress:save"

UserOverridden 표시: editOverridden==true → "★ Modified" (kLockdown)
```

#### renderPage4() — System

```
y=38:  "Batch: INC-001  Chicken"
y=55:  "Start: YYYY-MM-DD / 21 days"
y=75:  "WiFi: Connected  192.168.1.x"
        (cloudConnected==false: "WiFi: --" kTextDim)
y=92:  "Cloud: AWS IoT Connected"
        (cloudConnected==false: "Cloud: --" kTextDim)
y=112: "Boots: 12    Uptime: 6d 14h"
y=130: "FW: v1.0.0"
```

---

## 완료 기준 (Acceptance Criteria)

| # | 항목 | 기준 |
|---|---|---|
| AC-1 | 페이지 이동 | CW 회전 → page+1 (0→1→2→3→4→0 순환) |
| AC-2 | LongPress | 어느 페이지에서든 → Page 0 복귀 |
| AC-3 | Page 0 온도 | 실제 온도와 ±0.3°C 이내 표시 |
| AC-4 | Page 0 Fault | 센서 불량 시 "---" 표시 |
| AC-5 | 알람 표시 | tempAlarm==true → 온도값 kAlarmHigh 색상 |
| AC-6 | Footer 아이콘 | 히터 ON/OFF → HTR● 색상 변경 |
| AC-7 | Page 2 수동 | Click → AppController::applyCommand 호출 확인 |
| AC-8 | Page 3 편집 | editTempC 변경 → LongPress → plan.json 업데이트 |
| AC-9 | SafeMode | safeMode==true → 빨간 오버레이 표시 |
| AC-10 | 렌더러 격리 | MainUiRenderer에서 RuntimeState 직접 접근 없음 |
| AC-11 | 렌더 주기 | UiModel 변경 시에만 redraw, 최소 200ms 간격으로 과도한 LCD refresh 없음 |
