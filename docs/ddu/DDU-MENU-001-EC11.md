# DDU-MENU-001 — EC11 메뉴 시스템 전체 구현
> **Document ID**: DDU-MENU-001  
> **Version**: 1.0 | **Status**: Authoritative  
> **상위 문서**: 22_Incubator_DetailDesign.md, DDU-UI-001, DDU-PROV-001  
> **대상 AI**: Cursor  
> **목적**: EC11 엔코더 기반 전체 메뉴 시스템 구현.  
>           메인 화면 회전 → 메뉴 진입, 8개 메뉴 항목 탐색/실행,  
>           각 항목별 세부 UX와 위험 동작 보호 메커니즘까지 완전 명세.
>
> ⚠️ **Cursor 필독 — 절대 규칙**
> - `UiController`는 정책 조합만 담당. Device/Module 직접 제어 금지.
>   → 디바이스 테스트는 `ManualTestController` 별도 분리.
> - 모든 위험 동작(재부팅/공장초기화)은 `SystemActionService` 경유.
> - 메뉴 상태 머신은 `MenuNavigator` 로 분리, `UiController`에 포함시키지 않는다.
> - Tick 내 malloc/new/vTaskDelay 금지.
> - `incubator::` 네임스페이스 전용.
> - 공장초기화 10초 누르기는 Tick 기반 타이머로 구현 (Hardware ISR 아님).

---

## 1. 전체 메뉴 구조 확정

### 1.1 페이지 / 메뉴 체계

```
[MainScreen]  — EC11 회전으로 페이지 전환 (회전 = 페이지 이동)
  │
  ├─ Main Page 1        (온도/습도 대형 표시)
  ├─ Main Page 2        (부화 진행 현황)
  ├─ Main Page 3        (도움말)
  │
  └─ [버튼 길게 1.5초] → [MenuScreen] 진입
                    │
                    ├─ MENU 0: 부화 시작일 설정
                    ├─ MENU 1: 부화 프리셋 선택
                    ├─ MENU 2: 일별 부화 설정 편집
                    ├─ MENU 3: 수동 작동 테스트
                    ├─ MENU 4: WiFi 정보 리셋
                    ├─ MENU 5: BLE Provisioning
                    ├─ MENU 6: 시스템 재부팅
                    └─ MENU 7: 공장 초기화 (10초 롱클릭) + 경고문구 (정말 초기화 하시겠습니까?)
                    
                    [버튼 짧게] → 선택된 항목 실행
                    [버튼 길게 1.5초] → 메뉴 탈출 → MainScreen 복귀
```

> **설계 결정**: 
> MainScreen은 MainPage 1, Main Page 2, Main Page 3 세 화면만 EC11 회전으로 전환한다.  
> 나머지 기능은 모두 MenuScreen 하위 항목으로 진입.

### 1.2 메뉴 항목 인덱스 확정

| 인덱스 | 항목명 (표시) | 위험도 | 진입 방식 |
|---|---|---|---|---|
| 0 | 부화 시작일 설정 | 보통 | 버튼 짧게 → 날짜 편집 화면 |
| 1 | 부화 프리셋 선택 | 보통 | 버튼 짧게 → 프리셋 목록 |
| 2 | 일별 부화 설정 편집 | 보통 | 버튼 짧게 → 일별 부화 설정 목록 |
| 3 | 수동 작동 테스트 | 보통 | 버튼 짧게 → 수동 작동 테스트 화면 |
| 4 | WiFi 정보 리셋 | 높음 | 버튼 짧게 → 확인 요청 |
| 5 | BLE Provisioning | 보통 | 버튼 짧게 → QR 화면 |
| 6 | Reboot | 시스템 재부팅 | 높음 | 버튼 짧게 → 확인 요청 |
| 7 | FactoryReset | 공장 초기화 | 최고 | 버튼 **10초 유지** → 실행 |

---

## 2. MenuNavigator 설계

### 2.1 파일 위치

```
include/incubator/ui/MenuNavigator.h
src/ui/MenuNavigator.cpp
```

### 2.2 UiContext 열거형

```cpp
// include/incubator/ui/UiContext.h
#pragma once
#include <cstdint>

namespace incubator::ui
{
    enum class UiContext : uint8_t
    {
        MainScreen              = 0,   // PAGE 0/1/2, EC11 회전 = 페이지 전환
        MenuScreen              = 1,   // 메뉴 목록 탐색
        IncubationStartDateEdit = 2,   // MENU 0: 부화 시작일 설정
        PresetList              = 3,   // MENU 1: 프리셋 목록
        PlanRowEdit             = 4,   // MENU 2: 일별 부화 설정 편집
        ManualControl           = 5,   // MENU 3: 수동 작동 테스트
        WifiResetConfirm        = 6,   // MENU 4: WiFi 리셋 확인
        BleProvisioning         = 7,   // MENU 5: BLE QR 오버레이 (ProvisioningRenderer 위임)
        RebootConfirm           = 8,   // MENU 6: 시스템 재부팅
        FactoryReset            = 9,   // MENU 7: 공장초기화 (10초 버튼)
    };

    static constexpr uint8_t kMenuCount = 8U;
}
```
### 2.3 MenuNavigator 클래스 선언

```cpp
// include/incubator/ui/MenuNavigator.h
#pragma once
#include <incubator/ui/UiContext.h>
#include <incubator/ui/UiModel.h>
#include <cstdint>

namespace incubator::ui
{
    class MenuNavigator
    {
    public:
        // ── 외부에서 주입되는 콜백들 ────────────────────────────
        using ActionCallback = void(*)();  // 단순 실행 콜백

        struct Callbacks {
            ActionCallback onIncubationStartDateSave = nullptr;  // MENU 0 저장
            ActionCallback onPresetBatch             = nullptr;  // MENU 1 Preset 정보로 PlatRow 생성
            ActionCallback onPlanRowSave             = nullptr;  // MENU 2 PlanRow 저장
            ActionCallback onManualControl           = nullptr;  // MENU 3 실행
            ActionCallback onWifiReset               = nullptr;  // MENU 4 실행
            ActionCallback onBleStart                = nullptr;  // MENU 5 시작
            ActionCallback onReboot                  = nullptr;  // MENU 6 실행
            ActionCallback onFactoryReset            = nullptr;  // MENU 7 실행
        };

        explicit MenuNavigator(UiModel& model);
        void setCallbacks(const Callbacks& cb) { m_cb = cb; }

        // ── EC11 입력 처리 (UiController에서 호출) ─────────────
        void onEncoderDelta(int16_t delta);
        void onButtonShort();
        void onButtonLong();        // 1.5초
        void onButtonHold(uint32_t holdMs); // 매 Tick 누르고 있는 시간 전달

        // ── 상태 조회 ────────────────────────────────────────────
        UiContext   currentContext()  const { return m_context; }
        uint8_t     menuCursor()      const { return m_menuCursor; }
        bool        isInMenu()        const { return m_context != UiContext::MainScreen; }

        // ── 컨텍스트 전환 (내부 전용이나 테스트 가능하도록 public) ─
        void enterContext(UiContext ctx);
        void exitToMenu();      // 현재 → MenuScreen
        void exitToMain();      // 현재 → MainScreen

    private:
        UiModel&   m_model;
        UiContext  m_context     = UiContext::MainScreen;
        uint8_t    m_menuCursor  = 0U;

        // ── 컨텍스트별 상태 ──────────────────────────────────────
        // MENU 3, 6 확인 화면 커서 (0=YES, 1=NO)
        uint8_t    m_confirmCursor = 1U;  // 기본: NO (안전)

        // MENU 7 공장초기화 — 버튼 누르기 시간 추적
        uint32_t   m_holdStartMs  = 0U;
        bool       m_holdActive   = false;
        static constexpr uint32_t kFactoryResetHoldMs = 10000U;  // 10초

        Callbacks  m_cb{};

        // ── 각 컨텍스트 입력 핸들러 ──────────────────────────────
        void handleMain_Delta(int16_t delta);
        void handleMenu_Delta(int16_t delta);
        void handleMenu_Short();
        void handleConfirm_Delta(int16_t delta);
        void handleConfirm_Short(UiContext returnCtx, ActionCallback cb);
        void handleIncubationStartDate_Delta(int16_t delta);
        void handleIncubationStartDate_Short();
        void handlePresetList_Delta(int16_t delta);
        void handlePresetList_Short();
        void handlePlanRowEdit_Delta(int16_t delta);
        void handlePlanRowEdit_Short();
        void handleDeviceTest_Delta(int16_t delta);
        void handleDeviceTest_Short();
        void handleManual_Delta(int16_t delta);
        void handleManual_Short();
        void handleFactoryReset_Hold(uint32_t holdMs);

        // UiModel 동기화
        void syncMenuModel();
    };
}
```

### 2.4 enterContext() / exitToMain() 구현 가이드

```cpp
void MenuNavigator::enterContext(UiContext ctx)
{
    m_context = ctx;
    m_confirmCursor = 1U;  // 매번 NO로 초기화 (안전)
    syncMenuModel();
}

void MenuNavigator::exitToMain()
{
    m_context    = UiContext::MainScreen;
    m_menuCursor = 0U;
    m_holdActive = false;
    syncMenuModel();
}

void MenuNavigator::exitToMenu()
{
    m_context = UiContext::MenuScreen;
    syncMenuModel();
}
```

### 2.5 onButtonHold() — 공장초기화 10초 감지

```cpp
void MenuNavigator::onButtonHold(uint32_t holdMs)
{
    if (m_context != UiContext::FactoryReset) return;

    // UiModel에 진행률 갱신 (0~100%)
    uint8_t pct = static_cast<uint8_t>(
        (holdMs * 100U) / kFactoryResetHoldMs);
    m_model.factoryResetProgressPct = (pct > 100U) ? 100U : pct;

    if (holdMs >= kFactoryResetHoldMs && m_cb.onFactoryReset) {
        m_cb.onFactoryReset();
        // 실행 후 상태 리셋 (실제 재부팅이 이루어지므로 도달 안 할 수도 있음)
        exitToMain();
    }
}
```

---

## 3. UiModel 메뉴 필드

### 3.1 수정 파일

`include/incubator/ui/UiModel.h`

### 3.2 추가 필드

```cpp
// UiModel 구조체에 추가
struct UiModel
{
    // ... 기존 필드 유지 ...

    // ── 메뉴 네비게이션 ──────────────────────────────────────────
    UiContext currentContext;          // 현재 UI 컨텍스트
    uint8_t   menuCursor;             // 메뉴 목록 커서 (0~kMenuCount-1)
    uint8_t   confirmCursor;          // 확인 화면 커서 (0=YES, 1=NO)

    // ── 배치 시작일 편집 (MENU 0) ────────────────────────────────
    // 편집 중 임시값 (저장 전까지 UiModel에만 존재)
    uint16_t  editIncubationStartYear;      // 예: 2026
    uint8_t   editIncubationStartMonth;     // 1~12
    uint8_t   editIncubationStartDay;       // 1~31
    uint8_t   editBatchFieldCursor;         // 0=Year, 1=Month, 2=Day

    // ── 프리셋 편집 (MENU 1) ────────────────────────────────────
    uint8_t   presetListCursor;       // 선택된 Day 행 (0-based)
    // 기존 editDay, editTargetTempC, editTargetHumidityPct, editTurningEnabled 사용
    uint8_t   planEditFieldCursor;    // 0=Temp, 1=Humi, 2=Turning, 3=Interval

    // ── 수동 디바이스 테스트 / 수동 제어 (MENU 2, 5) ────────────
    uint8_t   deviceCursor;           // 0=Heater, 1=Humidifier, 2=Turner, 3=Fan, 
    bool      manualHeaterOn;
    bool      manualHumidifierOn;
    bool      manualTurnerOn;
    bool      manualFanOn;

    // ── 공장초기화 진행률 (MENU 8) ───────────────────────────────
    uint8_t   factoryResetProgressPct;  // 0~100

    // ── 메뉴 레이블 표시용 (렌더러가 읽는 상수 배열 인덱스) ──────
    // MenuRenderer가 kMenuLabels[menuCursor] 로 직접 참조
};
```

### 3.3 메뉴 레이블 상수 (MenuRenderer에서 사용)

```cpp
// include/incubator/ui/MenuLabels.h
#pragma once
#include <incubator/ui/UiContext.h>

namespace incubator::ui
{
    // kMenuCount = 8
    static const char* kMenuLabels[kMenuCount] = {
        "0. 부화 시작일 설정",
        "1. 부화 프리셋 선택",
        "2. 일별 부화 설정 편집",
        "3. 수동 작동 테스트",
        "4. WiFi 정보 리셋",
        "5. BLE Provisioning",
        "6. 시스템 재부팅",
        "7. 공장 초기화",
    };

    // 위험 항목 마킹 (렌더러가 색상 강조에 사용)
    static const bool kMenuDanger[kMenuCount] = {
        false,  // 부화 시작일
        false,  // 프리셋 선택
        false,  // 일별 부화 설정 편집
        false,  // 수동 작동 테스트
        true,   // WiFi 리셋 ← 위험
        false,  // BLE Provisioning
        true,   // 시스템 재부팅
        true,   // 공장초기화 ← 최고 위험
    };
}
```
---

## 4. ProductEventCode 메뉴 추가

`include/incubator/config/ProductEventCodes.h` 에 추가:

```cpp
// Menu / System (10600~10699)
MenuEntered           = 10600,  // 메뉴 진입 (Info)
MenuExited            = 10601,  // 메뉴 탈출 (Info)
IncubationStartDateChanged = 10602,  // 부화 시작일 변경 (Info)
PlanRowSaved          = 10603,  // PlanRow 저장 (Info)
WifiResetRequested    = 10604,  // WiFi 리셋 요청 (Warning)
WifiResetDone         = 10605,  // WiFi 리셋 완료 (Info)
ManualControlEntered  = 10606,  // 수동 제어 진입 (Warning)
ManualControlExited   = 10607,  // 수동 제어 종료 (Info)
RebootRequested       = 10608,  // 재부팅 요청 (Warning)
FactoryResetRequested = 10609,  // 공장초기화 요청 (Critical)
FactoryResetDone      = 10610,  // 공장초기화 완료 (Critical)
```

---

## 5. 메뉴 항목별 UX 상세 명세

### 5.1 MENU 0 — 부화 시작일 설정 (IncubationStartDateEdit)

**진입**: MenuScreen → MENU 0 → 버튼 짧게

**화면 레이아웃**:
```
┌──────────────────────────────────────────────────────────────┐
│ 부화 시작일 설정                                    [뒤로:길게] │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│       부화시작 년도   │2026 │                                  │
│       부화시작 월     │1 ~ 12│                                 │
│       부화시작 일     │1 ~ 31│                                 │
│                                                              │
│       [저장]    [취소]                                        │
│                                                              │
│          회전: 항목 이동                                       │
│          클릭: 편집 모드                                       │
│          회전: 값 증감                                         │
│          클릭: 원래 모드                                        │
└──────────────────────────────────────────────────────────────┘
```

**편집 상태 머신**:
```
fieldCursor=0 (Year)
  → 짧게: 편집모드 진입
  → 회전: editBatchYear ±1 (2020~2099 클램프)
  → 짧게: fieldCursor=1

fieldCursor=1 (Month)
  → 짧게: 편집모드 진입
  → 회전: editBatchMonth ±1 (1~12 wrap)
  → 짧게: fieldCursor=2

fieldCursor=2 (Day)
  → 짧게: 편집모드 진입
  → 회전: editBatchDay ±1 (1~해당 월 최대일 클램프)
  → 짧게: [저장] 버튼으로 이동
  
[저장] 실행 → IncubationStartDateChanged Event → exitToMenu()
[취소] 실행 → 편집 데이터 원복하고 → exitToMenu()

어느 필드에서든 길게: 취소 → exitToMenu()
```

**Day 유효성 검사**:
```cpp
// 윤년 처리 포함 월별 최대일 클램프
static uint8_t maxDayForMonth(uint16_t year, uint8_t month);
// 예: 2월 → 윤년이면 29, 아니면 28
```

---

### 5.2 MENU 1 — 부화 프리셑 선택 (PresetList → IncubationPlanTable 생성)

**진입**: MenuScreen → MENU 1 → 버튼 짧게 → **프리셑 선택** → IncubationPlanTable 생성

**PresetList 화면** (320×240):
```
┌──────────────────────────────────────────────────────────────┐
│ 프리셋 선택                                         [뒤로:길게] │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│  1   닭                           [ ]                        │
│  2   오리                         [ ]                        │
│ ...                                                         │
├──────────────────────────────────────────────────────────────┤
│  [짧게: 편집]  [길게: 메뉴 복귀]                               │
└──────────────────────────────────────────────────────────────┘
┌──────────────────────────────────────────────────────────────┐
│ 프리셋 선택                                         [취소:길게] │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│                                                              │
│  정말 다시 생성하겠습니까?                                       │
│  [ 예 ]     [아니오]                                          │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

**편집 필드 순서**: Temp(0) → Humi(1) → Turning(2) → Interval(3) → [저장]

**값 조정 스텝**:
| 필드 | 스텝 | 범위 |
|---|---|---|
| Temp | ±0.1°C | 20.0~45.0°C |
| Humi | ±1% | 30~95% |
| Turning | 토글 | ON/OFF |
| Interval | ±10min | 30~480min |

**저장**: `m_cb.onPresetRowSave` → `repo.savePlan()` → `PresetRowSaved` Event

---

### 5.3 MENU 2 — 일별 부화 설정 편집 (IncubationPlanList → IncubationPlanRowEdit)

**진입**: MenuScreen → MENU 1 → 버튼 짧게 → **프리셑 선택** → PlanRowList 생성

**PresetList 화면** (320×240):
```
┌──────────────────────────────────────────────────────────────┐
│ 일별 부화 설정                                      [뒤로:길게] │
├──────────────────────────────────────────────────────────────┤
│ Day  Temp   Humi  Turn  Interval  Override                   │
│  1   37.5   55%   ON    120min    [ ]                        │
│  2   37.5   55%   ON    120min    [ ]                        │
│▶ 3   37.5   55%   ON    120min    [ ]   ← 선택               │
│  4   37.8   58%   ON    120min    [ ]                        │
│  5   37.8   58%   ON    120min    [ ]                        │
│ ...  (최대 5행 표시, 스크롤)                                   │
├──────────────────────────────────────────────────────────────┤
│  [짧게: 편집]  [길게: 메뉴 복귀]                               │
└──────────────────────────────────────────────────────────────┘
```

**PlanRowEdit 화면**:
```
┌──────────────────────────────────────────────────────────────┐
│ Day 3 편집                                          [취소:길게] │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│  온도    [37.5]°C   ← 활성 필드 하이라이트                      │
│  습도    [ 55 ]%                                             │
│  전란    [ ON ]                                              │
│  간격    [120 ]min                                           │
│                                                              │
│  [저장]   [취소]                                              │
└──────────────────────────────────────────────────────────────┘
```

**편집 필드 순서**: Temp(0) → Humi(1) → Turning(2) → Interval(3) → [저장]

**값 조정 스텝**:
| 필드 | 스텝 | 범위 |
|---|---|---|
| Temp | ±0.1°C | 20.0~45.0°C |
| Humi | ±1% | 30~95% |
| Turning | 토글 | ON/OFF |
| Interval | ±min | 30~480min |

**저장**: `m_cb.onPresetRowSave` → `repo.savePlan()` → `PresetRowSaved` Event

---

### 5.4 MENU 3 — 수동 작동 테스트 (ManualControl)

**목적**: 부화 자동 제어를 **일시 정지**하고 릴레이/팬을 수동으로 직접 ON/OFF.  
실물 하드웨어 설치 후 배선 점검 목적.

**⚠️ 핵심 안전 규칙**:
```
진입 시: ManualControlEntered Event 발행
         ClimateControlModule은 이 Event 수신 시 제어 루프 일시 정지
         (SafeMode는 아님 — Relay 자체는 살아 있음)

종료 시: ManualControlExited Event 발행
         ClimateControlModule 제어 루프 재개
         모든 수동 상태 OFF (ManualTestController.releaseAll())
```

**화면** (MENU 2와 동일 레이아웃, 상단 "⚠ 수동 제어 중" 표시):
```
┌──────────────────────────────────────────────────────────────┐
│ ⚠ 수동 작동 테스트                                  [종료:길게] │
├──────────────────────────────────────────────────────────────┤
│  자동 온도/습도 제어가 일시 중단되었습니다.                     │
│                                                              │
│  ▶ [HEATER    ] OFF  → ON                                   │
│    [HUMIDIFIER] OFF                                          │
│    [FAN       ] OFF                                          │
│    [TURNER    ] OFF                                          │
│                                                              │
│   회전: 선택   짧게: ON/OFF 토글   길게: 종료 (자동 복구)      │
└──────────────────────────────────────────────────────────────┘
```
---

### 5.5 MENU 4 — WiFi 정보 리셋 (WifiResetConfirm)

**화면**:
```
┌──────────────────────────────────────────────────────────────┐
│ ⚠  WiFi 정보 리셋                                            │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│       저장된 WiFi 인증정보를 삭제합니다.                         │
│       삭제 후 BLE Provisioning이 필요합니다.                    │
│                                                              │
│          ┌────────┐         ┌─────┐                         │
│          │  아니오 │         │ 예  │   ← 기본: 아니오          │
│          └────────┘         └─────┘                         │
│            (하이라이트)                                       │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

**규칙**:
- 기본 커서: NO (오작동 방지)
- 회전: NO ↔ YES 토글
- 짧게: 선택 확정
  - YES 선택 → `m_cb.onWifiReset()` → `WifiResetDone` Event → exitToMenu()
  - NO 선택 → exitToMenu()
- 길게: 취소 → exitToMenu()

---

### 5.6 MENU 5 — BLE Provisioning

**동작**: `m_cb.onBleStart()` 즉시 호출 → `ProvisioningManager.requestMenuProvisioning()` → QR 오버레이 표시.  
`DDU-PROV-001` §9 UiController 연동 그대로 적용.  
취소: 버튼 길게 → `ProvisioningManager.cancelProvisioning()` → exitToMenu()

---

### 5.7 MENU 6 — 시스템 재부팅 (RebootConfirm)

MENU 3(WiFi 리셋)과 동일한 ConfirmScreen 패턴. 기본: NO.

YES 확정 → `m_cb.onReboot()` → `SystemActionService.requestReboot()` → `RebootRequested` Event → `esp_restart()`

---

### 5.8 MENU 7 — 공장 초기화 (FactoryReset, 10초 누르기)

**이 항목은 ConfirmScreen 없이 직접 10초 버튼 누르기로 실행한다.**  
(ConfirmScreen을 실수로 통과하는 경우를 막기 위해 의도적으로 진입장벽을 높임)

**화면**:
```
┌──────────────────────────────────────────────────────────────┐
│ ⛔ 공장 초기화                                                │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│   모든 설정, WiFi 정보, 부화 계획이 삭제됩니다.               │
│   실행하려면 버튼을 10초간 계속 누르세요.                      │
│                                                              │
│   ████████████████████░░░░░░░░░░  65%                       │
│   (버튼 누르는 동안 실시간 진행률 표시)                        │
│                                                              │
│   [버튼 뗌 → 취소]   [길게 1.5초 → 뒤로]                     │
└──────────────────────────────────────────────────────────────┘
```

**입력 처리**:
```
MenuNavigator.onButtonHold(holdMs) 호출 경로:
  UiController.tick()
    → buttonHeldMs = ec11.getButtonHeldMs()
      (버튼이 눌린 상태이면 ms 증가, 뗌 = 0으로 리셋)
    → navigator.onButtonHold(buttonHeldMs)
    → FactoryReset 컨텍스트: factoryResetProgressPct 갱신
    → 10000ms 도달 → m_cb.onFactoryReset()

버튼을 떼면 holdMs=0 → 진행률 0으로 리셋 (처음부터 다시)
```
---

## 6. UiController 수정 명세

### 6.1 의존성 추가

```cpp
// UiController 생성자에 추가
UiController(
    ...기존...,
    MenuNavigator&          navigator,
    ManualTestController&   manualTest,
    SystemActionService&    sysAction);
```

### 6.2 tick() 수정

```cpp
void UiController::tick()
{
    uint32_t now = m_clock.now();  // IClock 경유

    // 1. 엔코더 delta → MenuNavigator
    int16_t delta = m_ec11.consumeDelta();
    if (delta != 0) {
        m_navigator.onEncoderDelta(delta);
    }

    // 2. 버튼 상태 → MenuNavigator
    //    InputModule이 이미 debounce/longPress 처리 → Event로 수신
    //    (onEvent에서 처리, tick에서는 홀드 시간만 전달)
    if (m_buttonDown) {
        m_buttonHeldMs = now - m_buttonPressedAt;
        m_navigator.onButtonHold(m_buttonHeldMs);
    }

    // 3. SystemActionService tick (재부팅 예약 실행)
    m_sysAction.tick();

    // 4. ManualTestController → UiModel 동기화
    m_manualTest.syncToUiModel(m_uiModel);

    // 5. 센서값 → UiModel (기존 로직 유지)
    syncUiModel();
}
```

### 6.3 onEvent() 추가 처리

```cpp
// InputPressed (sourceId=Input_Encoder):
m_buttonDown      = false;
m_buttonHeldMs    = 0U;
m_buttonPressedAt = 0U;
m_navigator.onButtonShort();

// InputLongPress (sourceId=Input_Encoder):
m_buttonDown      = false;
m_buttonHeldMs    = 0U;
m_navigator.onButtonLong();

// InputStateChanged (pressed=true):
m_buttonDown      = true;
m_buttonPressedAt = event.timestamp;

// InputStateChanged (pressed=false, NOT longPress):
m_buttonDown      = false;
m_buttonHeldMs    = 0U;
// FactoryReset 컨텍스트라면 진행률 리셋
if (m_navigator.currentContext() == UiContext::FactoryReset) {
    m_uiModel.factoryResetProgressPct = 0U;
}

// ManualControlEntered:
m_uiModel.currentContext = UiContext::ManualControl;

// ManualControlExited:
m_manualTest.releaseAll();
```

### 6.4 MenuNavigator 콜백 바인딩

```cpp
// RegisterServices.cpp 또는 main.cpp에서 setup 시:
MenuNavigator::Callbacks cb;

cb.onBatchDateSave = []() {
    // UiModel의 편집값 → IncubationBatch → repo.saveBatch()
    g_batchSaveHandler();
};
cb.onPresetRowSave = []() {
    g_planRowSaveHandler();
};
cb.onWifiReset = []() {
    g_sysAction.resetWifiCredentials();
};
cb.onBleStart = []() {
    g_provMgr.requestMenuProvisioning();
};
cb.onReboot = []() {
    g_sysAction.requestReboot();
};
cb.onFactoryReset = []() {
    g_sysAction.factoryReset();
};

g_navigator.setCallbacks(cb);
```

---

## 7. 파일 목록 및 책임 요약

### 7.1 신규 생성 파일

| 파일 | 레이어 | 책임 |
|---|---|---|
| `include/incubator/ui/UiContext.h` | UI | 컨텍스트 열거형 |
| `include/incubator/ui/MenuLabels.h` | UI | 메뉴 레이블/위험도 상수 |
| `include/incubator/ui/MenuNavigator.h` | UI | 메뉴 상태 머신 선언 |
| `src/ui/MenuNavigator.cpp` | UI | 컨텍스트 전환, 입력 분기, 콜백 |
| `include/incubator/ui/MenuRenderer.h` | UI | 메뉴 화면 렌더러 선언 |
| `src/ui/MenuRenderer.cpp` | UI | 8개 컨텍스트 화면 구현 |
| `include/incubator/ui/ManualTestController.h` | UI/Policy | 릴레이/팬 수동 제어 |
| `src/ui/ManualTestController.cpp` | UI/Policy | 토글/해제 구현 |
| `include/incubator/infra/SystemActionService.h` | Infra | 시스템 액션 선언 |
| `src/infra/SystemActionService.cpp` | Infra | 재부팅/초기화 구현 |

### 7.2 수정 파일

| 파일 | 수정 내용 |
|---|---|
| `include/incubator/ui/UiModel.h` | 메뉴 필드 13개 추가 |
| `include/incubator/config/ProductEventCodes.h` | Menu/System 11개 Event 추가 (10600번대) |
| `include/incubator/ui/UiController.h` | MenuNavigator, ManualTestController, SystemActionService 주입 추가 |
| `src/ui/UiController.cpp` | tick() 홀드 타이머, onEvent() 버튼 상태 추적 수정 |
| `src/ui/MainUiRenderer.cpp` | render()에서 `isInMenu()` 시 MenuRenderer 위임 추가 |
| `src/composition/RegisterServices.cpp` | 신규 객체 AddModule 등록 |
| `src/main.cpp` | 신규 객체 선언, MenuNavigator 콜백 바인딩 |

### 7.3 의존 관계 요약

```
MenuNavigator
  ← depends on: UiModel
  ← depends on: Callbacks (함수 포인터 — 의존성 역전)
  ← used by: UiController (delta/button 이벤트 전달)
  ← used by: MainUiRenderer (isInMenu() 체크)

MenuRenderer
  ← depends on: UiModel (read-only)
  ← used by: MainUiRenderer.render() (컨텍스트 비 Main 시 위임)

ManualTestController
  ← depends on: RelayModule×4, IEventBus
  ← used by: MenuNavigator 콜백 경유 (UiController가 직접 호출하지 않음)
  ← used by: UiController.tick() syncToUiModel()

SystemActionService
  ← depends on: IEventBus, IClock, AppSettings, IIncubationPlanRepository
  ← used by: MenuNavigator 콜백 경유
  ← used by: UiController.tick() (reboot 지연 실행)
```

---

## 8. 구현 순서 (Cursor 작업 순서)

```
STEP 1. 타입/상수 정의
        UiContext.h 생성 (열거형 11개)
        MenuLabels.h 생성 (레이블 배열, 위험도 배열)
        UiModel.h 메뉴 필드 추가 + zero() 초기화
        ProductEventCodes.h Menu/System 11개 추가

STEP 2. SystemActionService 구현
        선언 + requestReboot() + resetWifiCredentials() + factoryReset()
        tick() 재부팅 지연 실행
        검증: Serial 로그 Event 확인

STEP 3. ManualTestController 구현
        선언 + toggleDevice() + releaseAll() + syncToUiModel()
        검증: 릴레이 ON/OFF 실제 동작 확인

STEP 4. MenuNavigator 구현
        4-1. enterContext() / exitToMain() / exitToMenu()
        4-2. onEncoderDelta() 분기 (컨텍스트별 핸들러)
        4-3. onButtonShort() 분기
        4-4. onButtonLong() 분기
        4-5. onButtonHold() — FactoryReset 타이머
        4-6. 모든 컨텍스트 핸들러 구현 (§6 UX 명세 기준)
        4-7. syncMenuModel()
        검증: Serial 로그로 컨텍스트 전환 시퀀스 확인

STEP 5. MenuRenderer 구현
        5-1. drawCommonHeader() / drawBackHint() 공통 헬퍼
        5-2. renderMenuList() (8항목, 위험 마킹)
        5-3. renderBatchDateEdit()
        5-4. renderPresetList() + renderPlanRowEdit()
        5-5. renderConfirm() (WiFi 리셋, 재부팅 공유)
        5-6. renderDeviceTest() + renderManualControl()
        5-7. renderFactoryReset() (진행률 바 포함)
        검증: 각 화면 TFT 출력 확인

STEP 6. UiController 수정
        6-1. 생성자에 Navigator/ManualTest/SysAction 주입
        6-2. tick() 홀드 타이머 + navigator.onButtonHold()
        6-3. onEvent() 버튼 상태 추적 (buttonDown, pressedAt)
        검증: EC11 회전 → 페이지 전환, 버튼 → 메뉴 진입

STEP 7. MainUiRenderer 수정
        render() 분기: isInMenu() → menuRenderer.render()
        
STEP 8. RegisterServices + main.cpp 연동
        객체 선언, Callback 바인딩, AddModule 등록

STEP 9. 통합 검증 (§13 AC 전체 체크)
```

---

## 9. 완료 기준 (Acceptance Criteria)

### AC-1. 기본 네비게이션

| 검증 항목 | 기준 |
|---|---|
| MainScreen CW 회전 | PAGE 0 → PAGE 1 → PAGE 2 전환 |
| MainScreen CCW 회전 | PAGE 2 → PAGE 1 → PAGE 0 전환 |
| 버튼 짧게 | MenuScreen 진입, MENU 0 하이라이트 |
| MenuScreen CW 회전 | 커서 0→1→...→7→0 wrap |
| MenuScreen CCW 회전 | 커서 0→7 wrap |
| 버튼 길게 1.5초 | MainScreen 복귀 |

### AC-2. 아키텍처 준수

| 검증 항목 | 기준 |
|---|---|
| MenuRenderer | RuntimeState 직접 접근 없음 |
| UiController | RelayModule.setTarget() 직접 호출 없음 |
| MenuNavigator | IDisplayDevice 접근 없음 |
| SystemActionService | UiModel 접근 없음 |
| Tick 내 금지 패턴 | malloc/new/vTaskDelay 없음 (factoryReset 제외) |

---

## 변경 이력

| 버전 | 날짜 | 내용 |
|---|---|---|
| 1.0 | 2026-05-04 | 초안 작성 — EC11 메뉴 시스템 전체 명세 |

---

*이 문서는 DDU-UI-001, DDU-PROV-001의 하위 구현 명세이다.  
충돌 시 우선순위: 00_PLATFORM_BIBLE > 22_Incubator_DetailDesign > DDU-UI-001 > 이 문서.*

---

## Implementation Update (2026-05-06)

- MENU 1 Preset confirmation defaults to YES so selecting the same species regenerates a fresh IncubationPlanTable SET from preset defaults.
- Applying a preset calls StartBatch and reloads the selected edit row from the regenerated plan immediately.
- MENU 5 BLE Provisioning is the only path that displays the QR screen; boot-time QR display is disabled.

## Implementation Note (2026-05-05)

FwCore.Common 제거 이후 V1 펌웨어는 별도 `MenuNavigator`/`MenuRenderer` 클래스로 분리하지 않고,
기존 `UiController`와 `MainUiRenderer` 안에 메뉴 상태를 통합한다.

- `longClick`: 홈에서 메뉴 진입, 메뉴/상세/Provisioning 화면에서 취소 또는 홈 복귀
- `click`: 메뉴 항목 실행 또는 편집 항목 선택
- 메뉴 항목은 본 문서의 8개 항목 순서를 유지한다.
- 현재 구현 완료 항목: `일별 설정`, `수동 테스트`, `BLE 설정`
- `WiFi 리셋`, `재부팅`, `공장 초기화`는 오동작 방지를 위해 확인 화면이 추가된 뒤 실제 동작을 연결한다.
