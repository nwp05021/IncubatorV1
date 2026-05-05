# DDU-UI-001 — Display Layout / EC11 Rotary / AHT20 Sensor Read
> **Document ID**: DDU-UI-001  
> **Version**: 1.0 | **Status**: Authoritative  
> **상위 문서**: 22_Incubator_DetailDesign.md (Incubator-DETAIL-001)  
> **목적**: Codex(AI 코드 생성)에게 전달하는 구현 단위 명세.  
>           화면 레이아웃 확정, EC11 로터리 SW 구현, AHT20 센서 읽어 화면 출력까지를  
>           단일 DDU 범위로 정의한다.
>
> ⚠️ **AI(Codex) 필독 — 최우선 규칙**
> - 이 문서와 충돌하는 코드를 생성하지 않는다.
> - FwCore 계층 규칙(Device/Module/Controller 분리)을 항상 준수한다.
> - ISR 내 복잡한 로직 금지, Tick 내 malloc/new/delay 금지.
> - `incubator::` 네임스페이스 전용 사용, FwCore 예약 sourceId(1~99) 사용 금지.

---

## 2026-05-05 구현 보정 사항

이 문서는 FwCore.Common 적용 시점의 초안이므로, 현재 `IncubatorV1` 구현은 아래처럼 보정한다.

- 화면 기준은 ST7789 물리 패널을 가로 회전한 **320 x 240 landscape** 이다. `St7789Display::setRotation(1)` 과 `UiLayout::kScreenW=320`, `kScreenH=240` 을 기준으로 한다.
- 사용자가 보는 방향은 오른쪽 90도 회전 기준이므로 실제 구현은 `St7789Display::setRotation(1)` 을 사용한다. 해상도는 계속 **320 x 240** 이다.
- 한글 표시는 필수 요구사항이다. `St7789Display::drawText()` 는 UTF-8 바이트를 감지하면 LovyanGFX `fonts::efontKR_16` 으로 전환하여 한글을 렌더링한다.
- 큰 온도/습도 숫자는 확대된 기본 폰트를 쓰지 않고 LovyanGFX 숫자 전용 `fonts::Font6` RLE 폰트를 `St7789Display::drawNumberText()` 로 출력한다.
- UI는 휴대폰 상단바처럼 `상태바(시간/알람/잠금/Cloud/WiFi 아이콘) + 제목 영역 + 본문 + 하단 조작 힌트` 구조로 렌더링한다.
- 현재 시각은 RTC/SNTP가 연결되기 전까지 `uptimeMs` 기반 `HH:MM` 표시를 사용한다. 실제 시각 동기화가 추가되면 `UiModel`에 epoch 또는 시각 문자열을 추가한다.
- EC11 회전은 홈 화면에서 페이지를 직접 넘기지 않는다. 홈은 제품의 기본 얼굴로 고정한다.
- `longClick`: 홈에서 메뉴 진입, 메뉴/상세 화면에서 홈 복귀. Plan 화면에서는 현재 편집값 저장 후 홈 복귀.
- `click`: 메뉴 항목 선택 또는 상세 화면의 선택/토글/편집 필드 전환.
- 구현 파일은 FwCore 경로가 아니라 현재 프로젝트 경로인 `include/ui/*`, `src/ui/*`, `include/devices/*`, `src/devices/*` 를 기준으로 한다.

---

## 목차

1. [범위 및 전제 조건](#1-범위-및-전제-조건)
2. [하드웨어 확정 사항](#2-하드웨어-확정-사항)
3. [화면 레이아웃 설계 (320×240 landscape)](#3-화면-레이아웃-설계-320240-landscape)
4. [Ec11InputDevice 구현 명세](#4-ec11inputdevice-구현-명세)
5. [Aht20 → 화면 출력 데이터 흐름](#5-aht20--화면-출력-데이터-흐름)
6. [UiController 구현 명세](#6-uicontroller-구현-명세)
7. [MainUiRenderer 구현 명세](#7-mainuirenderer-구현-명세)
8. [파일 목록 및 책임 요약](#8-파일-목록-및-책임-요약)
9. [구현 순서 (Codex 작업 순서)](#9-구현-순서-codex-작업-순서)
10. [완료 기준 (Acceptance Criteria)](#10-완료-기준-acceptance-criteria)

---

## 1. 범위 및 전제 조건

### 1.1 이 DDU의 범위

| 항목 | 포함 여부 | 비고 |
|---|---|---|
| 화면 레이아웃 확정 (5 Page) | ✅ | §3에서 픽셀 좌표까지 확정 |
| Ec11InputDevice (IInputDevice 구현) | ✅ | §4 |
| UiController (InputEvent → UiModel 갱신) | ✅ | §6 |
| MainUiRenderer (UiModel → IDisplayDevice 렌더) | ✅ | §7 |
| AHT20 센서값 → UiModel → 화면 출력 흐름 | ✅ | §5 |
| Aht20Driver / Aht20TempDevice / Aht20HumiDevice | ❌ | 이미 구현 완료 (전제) |
| St7789DisplayDevice | ❌ | 이미 구현 완료 (전제) |
| SensorModule (FwCore 표준) | ❌ | 이미 구현 완료 (전제) |
| ClimateControlModule / IncubationScheduleModule | ❌ | 별도 DDU |

### 1.2 전제 — 이미 구현된 것

```
✅ Esp32I2cBus          (include/incubator/hal/Esp32I2cBus.h)
✅ Aht20Driver          (include/incubator/devices/Aht20Driver.h)
✅ Aht20TempDevice      (include/incubator/devices/Aht20TempHumiDevices.h)
✅ Aht20HumiDevice      (include/incubator/devices/Aht20TempHumiDevices.h)
✅ St7789DisplayDevice  (include/incubator/devices/St7789DisplayDevice.h)
✅ DeviceRegistry       (src/devices/DeviceRegistry.cpp)
✅ SensorModule (Temp)  — AddSensor() 등록 완료
✅ SensorModule (Humi)  — AddSensor() 등록 완료
✅ DisplayModule        — AddDisplay() 등록 완료
✅ InputModule (stub)   — AddInput() 등록 완료 (Ec11InputDevice 미완)
✅ UiModel              (include/incubator/ui/UiModel.h) — 필드 완전 정의
```

### 1.3 이 DDU 완료 후 동작 목표

```
EC11 회전 → InputModule → InputStateChanged Event → UiController → activePage 변경
EC11 버튼 → InputModule → InputPressed/LongPress Event → UiController 처리
AHT20 읽기 → SensorModule → RuntimeState 갱신 → UiController → UiModel 갱신
DisplayModule Tick → MainUiRenderer.render() → TFT 화면에 온도·습도·페이지 표시
```

---

## 2. 하드웨어 확정 사항

### 2.1 디스플레이

| 항목 | 값 |
|---|---|
| 모델 | ST7789, GMT020-02-7P 2.0인치 |
| 핀 수 | 7핀 (별도 BL 핀 없음) |
| 물리 해상도 | 240 × 320 (세로) |
| 펌웨어 사용 해상도 | **320 × 240** (가로 회전, landscape) |
| 원점 (0,0) | 좌상단 |
| 라이브러리 | **LovyanGFX** |
| 구현 클래스 | `St7789DisplayDevice` (이미 완료) |

### 2.2 EC11 로터리 엔코더

| 항목 | 값 | 비고 |
|---|---|---|
| 핀 A (CLK) | `incubator::config::Pin::ENC_A` | PinConfig.h 단일 진실 |
| 핀 B (DT) | `incubator::config::Pin::ENC_B` | |
| 버튼 핀 | `incubator::config::Pin::ENC_BTN` | 내부 풀업, Active LOW |
| 인터페이스 | `fwcore::modules::input::IInputDevice` | |
| 구현 클래스 | `incubator::devices::Ec11InputDevice` | |
| 엔코더 방식 | **ISR + volatile 카운터** | Tick 내에서 카운터 소비 |
| 버튼 방식 | **폴링** (InputModule의 debounce/longPress 활용) | ISR 불필요 |

> **ISR 규칙**: ISR 내부에서는 `m_delta` 카운터 증감만 수행. EventBus 접근 금지.

### 2.3 AHT20 (I2C 센서)

| 항목 | 값 |
|---|---|
| 주소 | 0x38 |
| 버스 | `Esp32I2cBus` 공유 (DeviceRegistry 단일 인스턴스) |
| 읽기 방식 | non-blocking (Aht20Driver 캐시 기반) |
| 온도 Device | `Aht20TempDevice` → `SensorModule` (sourceId=1001) |
| 습도 Device | `Aht20HumiDevice` → `SensorModule` (sourceId=1002) |

---

## 3. 화면 레이아웃 설계 (320×240 landscape)

### 3.1 공통 레이아웃 구조

```
헨드폰 UI 참조
┌──────────────────────────── x=320 ──────────────────────────────┐
│ HEADER  y=0..23  (h=24)                                         │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  BODY                                                           │
│                                                                 │
│                   페이지별 콘텐츠                                 │
│                                                                 │
│                                                                 │
│                                                                 │
├─────────────────────────────────────────────────────────────────┤
│ FOOTER  y=215..239                                              │
└─────────────────────────────────────────────────────────────────┘
LongClick => 메뉴 화면, 서브 화면에서 LongClick => 복귀 
```

**좌표 상수 — 코드에서 직접 사용**:

```cpp
// include/incubator/ui/UiLayout.h
#pragma once
namespace incubator::ui::Layout
{
    // 전체 화면
    static constexpr int kScreenW = 320;
    static constexpr int kScreenH = 240;

    // Header 영역
    static constexpr int kHeaderY      = 0;
    static constexpr int kHeaderH      = 30;
    static constexpr int kHeaderTextY  = 8;   // 텍스트 기준선

    // Body 영역
    static constexpr int kBodyY        = 30;
    static constexpr int kBodyH        = 170;
    static constexpr int kBodyCenterY  = 115; // Body 수직 중앙

    // Footer 영역
    static constexpr int kFooterY      = 200;
    static constexpr int kFooterH      = 40;
    static constexpr int kFooterTextY  = 218; // Footer 텍스트 기준선

    // Footer 아이콘 x 좌표
    static constexpr int kIconHtrX     = 5;
    static constexpr int kIconHumX     = 55;
    static constexpr int kIconFanX     = 105;
    static constexpr int kIconTrnX     = 155;

    // Header 구분선
    static constexpr int kHeaderLineY  = 29;
    // Footer 구분선
    static constexpr int kFooterLineY  = 199;
}
```

### 3.2 색상 팔레트

```cpp
// include/incubator/ui/UiColors.h
#pragma once
#include <lgfx/v1/LGFXBase.hpp>  // TFT_xxx 색상 상수 원천

namespace incubator::ui::Color
{
    // LovyanGFX 16비트 RGB565 기반
    static constexpr uint32_t kBg          = TFT_BLACK;
    static constexpr uint32_t kHeader      = 0x1082U;  // 어두운 회색
    static constexpr uint32_t kFooter      = 0x1082U;
    static constexpr uint32_t kText        = TFT_WHITE;
    static constexpr uint32_t kTextDim     = TFT_DARKGREY;
    static constexpr uint32_t kAccentTemp  = TFT_ORANGE;
    static constexpr uint32_t kAccentHumi  = TFT_CYAN;
    static constexpr uint32_t kAlarmHigh   = TFT_RED;
    static constexpr uint32_t kAlarmLow    = TFT_BLUE;
    static constexpr uint32_t kOnIcon      = TFT_GREEN;
    static constexpr uint32_t kOffIcon     = TFT_DARKGREY;
    static constexpr uint32_t kSafeMode    = TFT_RED;
    static constexpr uint32_t kLockdown    = TFT_YELLOW;
    static constexpr uint32_t kDivider     = TFT_DARKGREY;
}
```

### 3.3 Main Page (기본 화면)

```
헨드폰 UI 참조

Main Page 1
┌─────────────────────────────────────────────────────────────────┐
│ [HH:MM] [Day 1/21]        [WiFi 아이콘][오류표시아이콘][부화진행율%] │ 
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│         38.5°C                          65%                     │
│      (온도 대형 표시)                (습도 대형 표시)               │  폰트 크기 4~5
│                                                                 │
│        최대한 크게 표시, 얼굴 UI 이므로 고급스럽게...                 │  
│                                                                 │
│       [작게 목표온도 표시]            [작게 목표습도 표시]           │
├─────────────────────────────────────────────────────────────────┤
│ [히터아이콘] [가습아이콘] [전란아이콘] [펜아이콘]  [1초 클릭으로 메뉴 진입]│  y=215~239
└─────────────────────────────────────────────────────────────────┘
=> Long Click 으로 메뉴 진입

Main Page 2
┌─────────────────────────────────────────────────────────────────┐
│ 상태 정보                                             YYYY-MM-DD │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  목표 온도    37.5°C           Hyst:   0.3                       │ 
│  목표 습도    60%              Hyst:   2%                        │ 
│  전란        ON                전란 간격    90min                 │  
│  다음 전란   00:45 후                                             │
│                                                                 │
│                                                                 │
├─────────────────────────────────────────────────────────────────┤
│ [히터아이콘] [가습아이콘] [전란아이콘] [펜아이콘]  [1초 클릭으로 메뉴 진입]│  y=215~239
└─────────────────────────────────────────────────────────────────┘

Main Page 3
┌─────────────────────────────────────────────────────────────────┐
│ SYSTEM                                               YYYY-MM-DD │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Uptime     12345s                                              │ 
│  Boot cnt   7                                                   │ 
│  Cloud      OFF                                                 │ 
│  IP         192.168.1.100                                       │ 
│  Batch      ACTIVE  (총 21일)                                   │ 
│  Lockdown   NO                                                  │ 
│  SafeMode   NO                                                  │ 
│                                                                 │
├─────────────────────────────────────────────────────────────────┤
│  히터●  가습●  전란●  펜● (좌측 상태 아이콘)          n일차  [=====] │  y=215~239
└─────────────────────────────────────────────────────────────────┘
```

---

## 4. Ec11InputDevice 구현 명세

### 4.1 파일 위치

```
include/incubator/devices/Ec11InputDevice.h
src/devices/Ec11InputDevice.cpp
```

### 4.2 인터페이스 계약

```cpp
// IInputDevice 계약 (FwCore)
// read() → true: 버튼이 현재 눌려 있음 (debounce는 InputModule이 처리)
// 엔코더 델타는 IInputDevice에 없으므로 확장 메서드로 노출

namespace incubator::devices
{
    class Ec11InputDevice final
        : public fwcore::modules::input::IInputDevice
    {
    public:
        // 생성자: 핀 번호를 PinConfig에서 받는다 (직접 GPIO 접근 금지 — init()에서만)
        Ec11InputDevice(gpio_num_t pinA,
                        gpio_num_t pinB,
                        gpio_num_t pinBtn);

        // ── IInputDevice 구현 ────────────────────────────
        bool init()   override;   // GPIO 설정 + ISR 등록
        bool read()   override;   // 버튼 raw 상태 반환 (true = 눌림, Active LOW 처리)

        // ── 엔코더 확장 API ──────────────────────────────
        // Tick 내에서 UiController가 직접 호출
        int16_t consumeDelta();   // 누적 delta 반환 후 0으로 리셋 (atomic)
        // consumeDelta > 0: CW(시계방향), < 0: CCW(반시계방향)

    private:
        gpio_num_t m_pinA;
        gpio_num_t m_pinB;
        gpio_num_t m_pinBtn;

        volatile int16_t m_delta = 0;   // ISR에서 증감, consumeDelta()에서 소비
        volatile uint8_t m_lastAB = 0;  // 이전 A/B 상태 (4-state Gray code)

        static void IRAM_ATTR isrHandler(void* arg);  // ISR: delta만 증감
    };
}
```

### 4.3 init() 구현 가이드

```cpp
bool Ec11InputDevice::init()
{
    // 1. GPIO 설정 — A, B 핀: 입력 + 풀업
    gpio_config_t io_conf_ab = {
        .pin_bit_mask = (1ULL << m_pinA) | (1ULL << m_pinB),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_ANYEDGE,  // A, B 엣지 모두 감지
    };
    if (gpio_config(&io_conf_ab) != ESP_OK) return false;

    // 2. 버튼 핀: 입력 + 풀업 (인터럽트 불필요 — 폴링)
    gpio_config_t io_conf_btn = {
        .pin_bit_mask = (1ULL << m_pinBtn),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    if (gpio_config(&io_conf_btn) != ESP_OK) return false;

    // 3. ISR 서비스 설치 (이미 설치된 경우 ESP_ERR_INVALID_STATE 무시)
    esp_err_t err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return false;

    // 4. A, B 핀에 ISR 등록
    gpio_isr_handler_add(m_pinA, isrHandler, this);
    gpio_isr_handler_add(m_pinB, isrHandler, this);

    // 5. 초기 A/B 상태 캡처
    m_lastAB = (uint8_t)((gpio_get_level(m_pinA) << 1)
                         | gpio_get_level(m_pinB));
    return true;
}
```

### 4.4 ISR 구현 가이드 (4-state Gray code 방식)

```cpp
// IRAM_ATTR 필수 — Flash 캐시 미스 방지
void IRAM_ATTR Ec11InputDevice::isrHandler(void* arg)
{
    // ⚠️ ISR 내 금지: malloc, new, vTaskDelay, EventBus 접근, printf
    auto* self = static_cast<Ec11InputDevice*>(arg);

    uint8_t a  = (uint8_t)gpio_get_level(self->m_pinA);
    uint8_t b  = (uint8_t)gpio_get_level(self->m_pinB);
    uint8_t ab = (uint8_t)((a << 1) | b);

    // 4-state transition table (Gray code)
    // 유효 전이: 00→01→11→10→00 (CW=+1) / 역방향 (CCW=-1)
    static const int8_t kTable[4][4] = {
    //   00   01   10   11  ← 현재
        { 0,  -1,  +1,   0 },  // 이전=00
        { +1,  0,   0,  -1 },  // 이전=01
        { -1,  0,   0,  +1 },  // 이전=10
        { 0,  +1,  -1,   0 },  // 이전=11
    };

    int8_t dir = kTable[self->m_lastAB][ab];
    if (dir != 0) {
        self->m_delta += dir;
    }
    self->m_lastAB = ab;
}
```

### 4.5 read() 구현

```cpp
bool Ec11InputDevice::read()
{
    // Active LOW: 눌리면 LOW → true 반환
    return (gpio_get_level(m_pinBtn) == 0);
}
```

### 4.6 consumeDelta() 구현

```cpp
int16_t Ec11InputDevice::consumeDelta()
{
    // ISR과 공유되므로 인터럽트 비활성화 후 읽기
    portDISABLE_INTERRUPTS();
    int16_t val = m_delta;
    m_delta = 0;
    portENABLE_INTERRUPTS();
    return val;
}
```

### 4.7 InputModule 옵션 설정

```cpp
// RegisterServices.cpp 에서
fwcore::api::options::InputOptions inputOpts;
inputOpts.moduleId      = incubator::config::ProductIds::Input_Encoder;
inputOpts.sourceId      = incubator::config::ProductIds::Input_Encoder;
inputOpts.pollIntervalMs = 10U;    // 10ms 폴링 (버튼 debounce용)
inputOpts.debounceMs    = 30U;
inputOpts.longPressMs   = 1500U;   // 길게 누름 1.5초
inputOpts.repeatMs      = 300U;
inputOpts.enabled       = true;

AddInput(services, inputOpts, g_ec11Device);
```

---

## 5. AHT20 → 화면 출력 데이터 흐름

### 5.1 전체 흐름

```
[Hardware: AHT20]
    ↓ I2C (Esp32I2cBus)
[Aht20Driver] — triggerMeasurement() / fetchResult()
    ↓ getCachedTemp() / getCachedHumi()
[Aht20TempDevice / Aht20HumiDevice]  : ISensorDevice
    ↓ read(SensorReading& out)
[SensorModule (Temp/Humi)]            : PollingModuleBase
    ↓ GetValue() / HasValue()
[UiController::onTick()]              : FwCore IEventSubscriber + custom tick
    ↓ runtimeState 갱신
    ↓ uiModel.displayTempC = runtimeState.currentTempC
[UiModel]
    ↓ (DisplayModule Tick → IDisplayRenderer::render() 호출)
[MainUiRenderer::render()]
    ↓ device.drawText() — 온도·습도 숫자 렌더
[St7789DisplayDevice]
    ↓ LovyanGFX
[TFT 화면]
```

### 5.2 UiController의 SensorModule 접근 방법

UiController는 직접 SensorModule 포인터를 보유하여 `getValue()` / `hasValue()`를 Tick마다 호출한다.  
EventBus를 통해 `SensorDisconnected` / `SensorOutOfRange` 이벤트를 수신하여 `tempSensorFault` / `humiSensorFault`를 갱신한다.

```cpp
// UiController.tick() 내부 로직 (개념)
void UiController::onTick()
{
    // 1. 센서값 갱신
    if (m_tempModule.hasValue()) {
        m_runtimeState.currentTempC = m_tempModule.getValue();
        m_runtimeState.tempSensorOk = true;
    }
    if (m_humiModule.hasValue()) {
        m_runtimeState.currentHumidityPct = m_humiModule.getValue();
        m_runtimeState.humiSensorOk = true;
    }

    // 2. 엔코더 델타 처리 (페이지 전환 등)
    int16_t delta = m_ec11Device.consumeDelta();
    if (delta != 0) handleEncoderDelta(delta);

    // 3. UiModel 동기화
    syncUiModel();
}
```

### 5.3 센서 오류 시 화면 처리

| 상태 | UiModel 필드 | 화면 표현 |
|---|---|---|
| 정상 | `tempSensorFault=false` | 온도 숫자 kAccentTemp |
| 센서 연결 끊김 | `tempSensorFault=true` | "---" 표시, kAlarmHigh |
| 범위 초과 알람 | `tempAlarm=true` | 숫자 kAlarmHigh + 깜빡임 |
| 범위 미만 알람 | `tempAlarm=true` | 숫자 TFT_BLUE + 깜빡임 |

> 깜빡임: `uptimeMs / 500 % 2 == 0` 조건으로 `drawText` skip

---

## 6. UiController 구현 명세

### 6.1 파일 위치

```
include/incubator/ui/UiController.h
src/ui/UiController.cpp
```

### 6.2 책임

- FwCore `IEventSubscriber` 구현 → SensorModule Event 수신
- 엔코더 delta 소비 → 페이지 전환 / 편집 모드 값 조정
- `RuntimeState` → `UiModel` 동기화
- Plan Edit 상태 머신 관리 (EditState)
- 완료 기준: DisplayModule/Renderer와 완전히 분리됨

### 6.3 클래스 선언

```cpp
// include/incubator/ui/UiController.h
#pragma once
#include <fwcore/core/IEventBus.h>
#include <fwcore/modules/sensor/SensorModule.h>
#include <incubator/ui/UiModel.h>
#include <incubator/domain/model/RuntimeState.h>
#include <incubator/devices/Ec11InputDevice.h>

namespace incubator::ui
{
    // EditState: Plan Edit 내부 상태
    enum class EditState : uint8_t
    {
        None        = 0,  // 비편집 모드
        DaySelect   = 1,  // Day 선택 중
        FieldTemp   = 2,  // 온도 조정 중
        FieldHumi   = 3,  // 습도 조정 중
        FieldTurn   = 4,  // 전란 토글 중
    };

    // 페이지 수
    static constexpr uint8_t kPageCount = 5U;

    class UiController final
        : public fwcore::core::IEventSubscriber
    {
    public:
        UiController(
            fwcore::core::IEventBus&            eventBus,
            fwcore::modules::sensor::SensorModule& tempModule,
            fwcore::modules::sensor::SensorModule& humiModule,
            incubator::devices::Ec11InputDevice&   ec11,
            incubator::domain::model::RuntimeState& runtimeState,
            UiModel&                               uiModel);

        void setup();   // EventBus 구독 등록
        void tick();    // Kernel Tick 3단계에서 호출 (AddModule 또는 직접)

        // IEventSubscriber
        void onEvent(const fwcore::core::SystemEvent& event) override;

        // Plan Edit 콜백 (RegisterServices에서 설정)
        using PatchCallback = void(*)(uint16_t day,
                                      float tempC,
                                      float humidityPct,
                                      bool turningEnabled);
        void setPatchCallback(PatchCallback cb) { m_patchCb = cb; }

    private:
        void handleEncoderDelta(int16_t delta);
        void handleButtonPress();
        void handleButtonLongPress();
        void syncUiModel();

        fwcore::core::IEventBus&                 m_eventBus;
        fwcore::modules::sensor::SensorModule&   m_tempModule;
        fwcore::modules::sensor::SensorModule&   m_humiModule;
        incubator::devices::Ec11InputDevice&     m_ec11;
        incubator::domain::model::RuntimeState&  m_runtimeState;
        UiModel&                                 m_uiModel;

        EditState m_editState   = EditState::None;
        uint8_t   m_manualCursor = 0U;  // PAGE 3: 현재 선택 버튼 (0~3)
        PatchCallback m_patchCb = nullptr;

        // 알람 깜빡임용
        bool      m_blinkVisible = true;
        uint32_t  m_lastBlinkMs  = 0U;
    };
}
```

### 6.4 페이지 전환 규칙

```
비편집 모드 (editState == None):
  엔코더 CW (+1) → activePage = (activePage + 1) % kPageCount
  엔코더 CCW (-1) → activePage = (activePage + kPageCount - 1) % kPageCount

편집 모드 (PAGE 3 Manual):
  엔코더 CW/CCW → manualCursor 이동 (0~3 wrap)
  버튼 짧게 → 현재 커서 항목 ON/OFF 토글

편집 모드 (PAGE 4 Plan Edit) — §3.6 상태 머신 참조
```

### 6.5 onEvent() 처리 목록

```
EventCode::SensorDisconnected  (sourceId=Sensor_Temp) → tempSensorFault=true
EventCode::SensorRecovered     (sourceId=Sensor_Temp) → tempSensorFault=false
EventCode::SensorDisconnected  (sourceId=Sensor_Humi) → humiSensorFault=true
EventCode::SensorRecovered     (sourceId=Sensor_Humi) → humiSensorFault=false
EventCode::SensorOutOfRange    (sourceId=Sensor_Temp) → tempAlarm=true
EventCode::SensorOutOfRange    (sourceId=Sensor_Humi) → humiAlarm=true
ProductEventCode::TempAlarmCleared                    → tempAlarm=false
ProductEventCode::HumidAlarmCleared                   → humiAlarm=false
EventCode::SafeModeEntered                            → safeMode=true
EventCode::SafeModeExited                             → safeMode=false
ProductEventCode::LockdownStarted                     → lockdownActive=true
InputPressed                   (sourceId=Input_Encoder) → handleButtonPress()
InputLongPress                 (sourceId=Input_Encoder) → handleButtonLongPress()
```

---

## 7. MainUiRenderer 구현 명세

### 7.1 파일 위치

```
include/incubator/ui/MainUiRenderer.h
src/ui/MainUiRenderer.cpp
```

### 7.2 책임

- `IDisplayRenderer` 구현
- `UiModel`만 읽음 (RuntimeState 직접 접근 금지)
- 페이지별 렌더 함수 분리 (renderPage1~5)
- Header, Footer는 공통 함수로 분리

### 7.3 클래스 선언

```cpp
// include/incubator/ui/MainUiRenderer.h
#pragma once
#include <fwcore/modules/display/IDisplayRenderer.h>
#include <fwcore/modules/display/IDisplayDevice.h>
#include <incubator/ui/UiModel.h>
#include <incubator/ui/UiLayout.h>
#include <incubator/ui/UiColors.h>

namespace incubator::ui
{
    class MainUiRenderer final
        : public fwcore::modules::display::IDisplayRenderer
    {
    public:
        explicit MainUiRenderer(const UiModel& model);

        // IDisplayRenderer
        void render(fwcore::modules::display::IDisplayDevice& device) override;

    private:
        const UiModel& m_model;

        void renderHeader(fwcore::modules::display::IDisplayDevice& d);
        void renderFooter(fwcore::modules::display::IDisplayDevice& d);
        void renderSafeMode(fwcore::modules::display::IDisplayDevice& d);
        void renderPage1(fwcore::modules::display::IDisplayDevice& d);  // Main
        void renderPage2(fwcore::modules::display::IDisplayDevice& d);  // Status
        void renderPage3(fwcore::modules::display::IDisplayDevice& d);  // Manual
        void renderPage4(fwcore::modules::display::IDisplayDevice& d);  // Plan Edit
        void renderPage5(fwcore::modules::display::IDisplayDevice& d);  // System

        // 공통 헬퍼
        void drawStatusIcon(fwcore::modules::display::IDisplayDevice& d,
                            int x, int y, const char* label, bool on);
        void drawAlarmBlink(fwcore::modules::display::IDisplayDevice& d,
                            int x, int y, const char* text, int size);
        void formatTemp(char* buf, size_t len, float tempC);
        void formatHumi(char* buf, size_t len, float humi);
        void formatTime(char* buf, size_t len, uint32_t uptimeMs);
    };
}
```

### 7.4 render() 진입 흐름

```cpp
void MainUiRenderer::render(IDisplayDevice& device)
{
    device.beginFrame();
    device.clear();

    if (m_model.safeMode) {
        renderSafeMode(device);
        renderFooter(device);   // Footer는 SafeMode에서도 표시
        device.endFrame();
        return;
    }

    renderHeader(device);

    switch (m_model.activePage) {
        case 0: renderPage1(device); break;
        case 1: renderPage2(device); break;
        case 2: renderPage3(device); break;
        case 3: renderPage4(device); break;
        case 4: renderPage5(device); break;
        default: renderPage1(device); break;
    }

    renderFooter(device);
    device.endFrame();
}
```

### 7.5 drawStatusIcon() 구현 가이드

```cpp
void MainUiRenderer::drawStatusIcon(IDisplayDevice& d,
                                    int x, int y,
                                    const char* label, bool on)
{
    // "HTR●" 형태 — ● 색상으로 ON/OFF 표현
    char buf[8];
    snprintf(buf, sizeof(buf), "%s", label);
    d.drawText(x, y, buf);
    // ● 색상은 IDisplayDevice 확장 API 또는 St7789DisplayDevice cast
    // 현재 IDisplayDevice에 color 매개변수 없으므로:
    // → St7789DisplayDevice에 setTextColor() 확장 후 렌더러에서 캐스트 사용
    //   또는 ON="HTR+" / OFF="HTR-" 텍스트 구분으로 임시 대응
}
```

> ⚠️ **IDisplayDevice 색상 확장 결정**: `St7789DisplayDevice`에 `setTextColor(uint32_t)` / `setFontSize(uint8_t)` 메서드를 추가하고, `MainUiRenderer`에서 `static_cast<St7789DisplayDevice&>(device)`로 접근한다. 이는 Product Layer 내부에서만 허용되는 패턴이다. FwCore IDisplayDevice 인터페이스 변경 금지.

### 7.6 formatTemp() / formatHumi()

```cpp
void MainUiRenderer::formatTemp(char* buf, size_t len, float tempC)
{
    if (!m_model.tempSensorFault) {
        snprintf(buf, len, "%.1fC", (double)tempC);
    } else {
        snprintf(buf, len, "---");
    }
}

void MainUiRenderer::formatHumi(char* buf, size_t len, float humi)
{
    if (!m_model.humiSensorFault) {
        snprintf(buf, len, "%.0f%%", (double)humi);
    } else {
        snprintf(buf, len, "---");
    }
}
```

---

## 8. 파일 목록 및 책임 요약

### 8.1 신규 생성 파일

| 파일 | 레이어 | 책임 |
|---|---|---|
| `include/incubator/ui/UiLayout.h` | UI | 좌표 상수 정의 |
| `include/incubator/ui/UiColors.h` | UI | 색상 상수 정의 |
| `include/incubator/devices/Ec11InputDevice.h` | Device | IInputDevice 구현 선언 |
| `src/devices/Ec11InputDevice.cpp` | Device | ISR, init, read, consumeDelta 구현 |
| `include/incubator/ui/UiController.h` | UI | RuntimeState→UiModel 변환, EC11 처리 |
| `src/ui/UiController.cpp` | UI | tick(), onEvent(), editState 머신 |
| `include/incubator/ui/MainUiRenderer.h` | UI | IDisplayRenderer 구현 선언 |
| `src/ui/MainUiRenderer.cpp` | UI | 5페이지 렌더, 공통 Header/Footer |

### 8.2 수정 파일

| 파일 | 수정 내용 |
|---|---|
| `include/incubator/devices/St7789DisplayDevice.h` | `setTextColor(uint32_t)`, `setFontSize(uint8_t)` 확장 메서드 추가 |
| `src/devices/DeviceRegistry.cpp` | `g_ec11Device` 인스턴스 추가, `init()` 호출 추가 |
| `src/composition/RegisterServices.cpp` | `AddInput()` 옵션에 `Ec11InputDevice` 연결, `UiController` 및 `MainUiRenderer` 생성 및 `AddDisplay()` 렌더러 연결 |
| `src/main.cpp` | UiController의 `setup()` 호출, `tick()` 연결 확인 |

### 8.3 의존 관계 요약

```
Ec11InputDevice
  ← depends on: driver/gpio.h (ESP-IDF), fwcore::modules::input::IInputDevice
  ← used by: UiController (consumeDelta), InputModule (read/debounce)

UiController
  ← depends on: SensorModule (getvalue), Ec11InputDevice (consumeDelta),
                RuntimeState, UiModel, IEventBus
  ← used by: main.cpp tick loop (직접 tick() 호출 또는 AddModule)

MainUiRenderer
  ← depends on: UiModel (read-only), UiLayout, UiColors, St7789DisplayDevice (cast)
  ← used by: DisplayModule (IDisplayRenderer::render() 호출)
```

---

## 9. 구현 순서 (Codex 작업 순서)

```
STEP 1. UiLayout.h + UiColors.h 생성
        — 좌표 상수, 색상 상수만 정의. 로직 없음.

STEP 2. Ec11InputDevice.h + Ec11InputDevice.cpp 구현
        — init(): GPIO 설정, ISR 등록
        — isrHandler(): 4-state Gray code delta 증감
        — read(): 버튼 Active LOW
        — consumeDelta(): portDISABLE/ENABLE_INTERRUPTS 사용
        — 검증: serial에서 delta, button read 출력

STEP 3. St7789DisplayDevice 확장
        — setTextColor(uint32_t), setFontSize(uint8_t) 추가
        — LovyanGFX gfx.setTextColor(), gfx.setTextSize() 위임

STEP 4. DeviceRegistry.cpp 수정
        — g_ec11Device 추가 및 init()

STEP 5. UiController.h + UiController.cpp 구현
        — setup(): EventBus 구독
        — tick(): consumeDelta → handleEncoderDelta, syncUiModel
        — onEvent(): 센서/입력 이벤트 처리
        — editState 머신 (PAGE 4)
        — 검증: 회전 시 activePage 변경, 센서값 UiModel 반영 확인

STEP 6. MainUiRenderer.h + MainUiRenderer.cpp 구현
        — render() 진입 흐름
        — renderHeader(), renderFooter() 공통 구현
        — renderPage1() (Main) 우선 구현 → 화면 출력 확인
        — renderPage2~5() 순차 구현
        — SafeMode 오버레이

STEP 7. RegisterServices.cpp + main.cpp 연결
        — AddInput() + Ec11InputDevice
        — AddDisplay() + MainUiRenderer 연결
        — UiController tick() 연결

STEP 8. 통합 검증 (§10 완료 기준 체크)
```

---

## 10. 완료 기준 (Acceptance Criteria)

### AC-1. EC11 입력 동작

| 검증 항목 | 기준 |
|---|---|
| CW 1클릭 | activePage +1 (5→0 wrap) |
| CCW 1클릭 | activePage -1 (0→4 wrap) |
| 버튼 짧게 누름 | InputPressed Event 발행 확인 (Serial TraceLog) |
| 버튼 1.5초 누름 | InputLongPress Event 발행 확인 |
| ISR 안전성 | 빠른 회전 시 delta 손실 없음 (±5 이내 오차) |

### AC-2. AHT20 → 화면 출력

| 검증 항목 | 기준 |
|---|---|
| 온도 표시 | 실제 온도와 ±0.3°C 이내 |
| 습도 표시 | 실제 습도와 ±2% 이내 |
| 갱신 주기 | SensorModule pollInterval 준수 (최소 1초) |
| 센서 오류 | AHT20 연결 해제 시 "---" 표시 |

### AC-3. 화면 레이아웃

| 검증 항목 | 기준 |
|---|---|
| PAGE 1 Main | 온도·습도 대형 숫자, 목표값, 아이콘 4개 표시 |
| Header | 페이지 레이블, Day, 시각 상시 표시 |
| Footer | HTR/HUM/FAN/TRN 상태 아이콘 상시 표시 |
| SafeMode | 어떤 페이지에서든 SafeMode 오버레이 표시 |
| 경계 조건 | SafeMode=false, tempSensorFault=true 시 "---" 표시 |

### AC-4. 아키텍처 준수

| 검증 항목 | 기준 |
|---|---|
| MainUiRenderer | RuntimeState 직접 접근 코드 없음 |
| Ec11InputDevice | EventBus 접근 코드 없음 |
| UiController | Device 직접 제어 코드 없음 (ec11.consumeDelta() 제외) |
| Tick 내 금지 패턴 | malloc / new / vTaskDelay 없음 |
| ISR 내 금지 패턴 | EventBus, printf, malloc 없음 |

---

## 변경 이력

| 버전 | 날짜 | 내용 |
|---|---|---|
| 1.0 | 2026-05-03 | 초안 작성 — 화면 레이아웃 확정, EC11/AHT20 DDU 작성 |

---

*이 문서는 FwCore.Common 00_PLATFORM_BIBLE.md 및 22_Incubator_DetailDesign.md의 하위 문서이다.  
충돌 시 우선순위: 00_PLATFORM_BIBLE > 22_Incubator_DetailDesign > 이 문서.*

---

## Implementation Note (2026-05-05)

V1 펌웨어의 기준 해상도는 320x240 landscape이다.
상태바는 좌측 시각, 우측 LOCAL/CLOUD 및 WiFi 신호 아이콘을 표시하고,
BLE Provisioning 중에는 전체 화면 QR 오버레이를 우선 표시한다.

큰 온도/습도 숫자는 일반 텍스트 확대가 아니라 LovyanGFX 숫자 전용 폰트 경로를 사용한다.
한글 라벨은 ST7789 렌더러의 UTF-8 한글 폰트 경로를 사용한다.
