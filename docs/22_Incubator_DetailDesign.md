# Incubator — Detail Design
> **Document ID**: Incubator-DETAIL-001
> **Version**: 2.0 | **Status**: Authoritative
> **상위 문서**: 21_Incubator_ProductSpec.md (Incubator-SPEC-001)
> **목적**: ProductSpec에서 미결로 남겨진 설계 결정 사항을 확정하고,
>           구현 전 기준 문서로 삼는다.
>
> ⚠️ 이 문서의 결정은 ProductSpec보다 구체적이며, 구현 코드와 일치해야 한다.
> 이 문서가 ProductSpec과 충돌하면 **이 문서가 우선**한다.
> 단, 두 문서 모두 00_PLATFORM_BIBLE.md에는 종속된다.

---

## 목차

1. [I2C HAL 공유 전략](#1-i2c-hal-공유-전략)
2. [AHT20 Device 공유 구조](#2-aht20-device-공유-구조)
3. [ClimateControl 알람 임계값](#3-climatecontrol-알람-임계값)
4. [TurningControl 정책](#4-turningcontrol-정책)
5. [AppSettings 완전 필드 정의](#5-appsettings-완전-필드-정의)
6. [RuntimeState 완전 필드 정의](#6-runtimestate-완전-필드-정의)
7. [UiModel 완전 필드 정의](#7-uimodel-완전-필드-정의)
8. [LocalPlanRepository 저장 전략](#8-localplanrepository-저장-전략)
9. [히스테리시스 및 PID 전환 전략](#9-히스테리시스-및-pid-전환-전략)
10. [Product EventCode — FwCore 연동 방식](#10-product-eventcode--fwcore-연동-방식)
11. [Display 하드웨어 및 라이브러리 결정](#11-display-하드웨어-및-라이브러리-결정)
12. [Lockdown 구간 정책](#12-lockdown-구간-정책)
13. [미결 사항 및 향후 검토 항목](#13-미결-사항-및-향후-검토-항목)

---

## 1. I2C HAL 공유 전략

### 1.1 결정 배경

I2C 버스를 각 Device가 직접 GPIO를 할당하거나 Arduino `Wire`를 중복 초기화할 경우
초기화 경쟁으로 재부팅 문제가 실제로 경험됨. 따라서 **I2C 버스는 HAL 계층에서 단 한 번 초기화**하고,
모든 Device는 주입된 `II2cBus` 참조를 공유한다. Incubator 구현은 **ESP-IDF I2C 마스터 드라이버** 기반이다.

### 1.2 II2cBus 인터페이스 (FwCore HAL)

```cpp
// FwCore.Common: include/fwcore/hal/II2cBus.h
namespace fwcore::hal
{
    class II2cBus
    {
    public:
        virtual ~II2cBus() = default;

        // SDA/SCL·클럭 설정. 부팅 시 1회만 호출.
        virtual bool init(int sdaPin, int sclPin, uint32_t freqHz = 400000U) = 0;

        // 레지스터 없는 Raw 전송 (AHT20 스타일)
        virtual bool write(uint8_t addr, const uint8_t* data, size_t len) = 0;
        virtual bool read (uint8_t addr, uint8_t* buf,  size_t len) = 0;

        // 레지스터 기반 전송 (일반 센서)
        virtual bool writeReg(uint8_t addr, uint8_t reg,
                              const uint8_t* data, size_t len) = 0;
        virtual bool readReg (uint8_t addr, uint8_t reg,
                              uint8_t* buf,  size_t len) = 0;

        virtual bool isReady(uint8_t addr) const = 0;  // ACK 확인
    };
}
```

### 1.3 Esp32I2cBus 구현 위치

```
include/incubator/hal/Esp32I2cBus.h   ← II2cBus 구현 선언
src/hal/Esp32I2cBus.cpp               ← ESP-IDF i2c_master_bus / 장치 핸들 기반
```

### 1.4 DeviceRegistry에서의 초기화 순서

개념 순서는 아래와 같다. **전문은 `src/devices/DeviceRegistry.cpp`가 단일 진실**이다.

```cpp
// src/devices/DeviceRegistry.cpp — 개념 요약
void DeviceRegistry::init()
{
    (void)g_i2c.init(incubator::config::Pin::I2C_SDA,
                     incubator::config::Pin::I2C_SCL, 400000U);

    (void)g_aht20.init();   // 생성자에 II2cBus& 이미 주입됨
    (void)g_temp.init();
    (void)g_humi.init();

    (void)g_heater.init();
    (void)g_humidifier.init();
    (void)g_turner.init();
    (void)g_fan.reinitialize();
    (void)g_fan.setEnabled(false);
    (void)g_display.init();
    (void)g_encoder.init();
}
```

### 1.5 규칙

```
✅ II2cBus 인스턴스는 DeviceRegistry에서 단 1개 생성, static 수명
✅ 모든 I2C Device는 생성자에서 II2cBus& 참조를 받는다
❌ Arduino Wire / Wire.begin() 직접 호출 금지
❌ GPIO 핀 번호를 Device 생성자 인자로 전달 금지 (I2C 주소만 허용)
```

---

## 2. AHT20 Device 공유 구조

### 2.1 결정

AHT20은 I2C 버스에 1개만 존재하지만 FwCore는 온도/습도를 별개 `ISensorDevice`로 관리한다.
공유 드라이버 패턴으로 해결한다.

### 2.2 구조

```
Aht20Driver  (실제 I2C 통신 담당, static 수명)
    │  II2cBus& 참조 보유
    │  read() 호출 시 온도+습도 동시 측정 후 내부 캐시 갱신
    │
    ├── Aht20TempDevice  : ISensorDevice
    │       read() → m_driver.getCachedTemp()
    │
    └── Aht20HumiDevice  : ISensorDevice
            read() → m_driver.getCachedHumi()
```

### 2.3 Aht20Driver 인터페이스

단일 진실은 `include/incubator/devices/Aht20Driver.h`이다. 아래는 공개 면만 발췌한다.

```cpp
// include/incubator/devices/Aht20Driver.h — 발췌
namespace incubator::devices
{
    struct Aht20Reading
    {
        float    tempC        = 0.0f;
        float    humidityPct  = 0.0f;
        bool     valid        = false;
        uint32_t timestampMs  = 0U;
    };

    class Aht20Driver
    {
    public:
        static constexpr uint8_t DefaultAddress = 0x38U;

        explicit Aht20Driver(fwcore::hal::II2cBus& bus,
                             uint8_t address = DefaultAddress);

        bool init();
        bool triggerMeasurement();
        bool fetchResult();

        float    getCachedTemp()     const { return m_cached.tempC; }
        float    getCachedHumi()     const { return m_cached.humidityPct; }
        bool     isCacheValid()      const { return m_cached.valid; }
        uint32_t getCacheTimestamp() const { return m_cached.timestampMs; }
        bool     isConnected()       const { return m_ok; }
        bool     reinitialize();

    private:
        fwcore::hal::II2cBus& m_bus;
        uint8_t               m_address;
        bool                  m_ok             = false;
        bool                  m_measurePending = false;
        Aht20Reading          m_cached         = {};
    };
}
```

### 2.4 Tick 내 동작 흐름

```
SensorModule(Temp).tick()
  → Aht20TempDevice.read()
      → driver.triggerMeasurement()  ← 아직 측정 안 됐으면 요청
      → driver.fetchResult()         ← 완료됐으면 캐시 갱신
      → out.value = driver.getCachedTemp()

SensorModule(Humi).tick()
  → Aht20HumiDevice.read()
      → driver.fetchResult()         ← 이미 캐시된 값 사용
      → out.value = driver.getCachedHumi()
```

> **규칙**: Tick 1회에 I2C 버스 access는 1회만 발생한다. 두 번째 read()는 캐시 반환.

---

## 3. ClimateControl 알람 임계값

### 3.1 결정

알람 임계값은 **AppSettings에 저장**한다. UI에서 사용자가 변경 가능하며, NVS에 영속된다.
기본값은 아래와 같다.

### 3.2 기본값 테이블

| 항목 | 필드명 | 기본값 | 알람 등급 |
|---|---|---|---|
| 온도 상한 경보 오프셋 | `tempAlarmHighOffsetC` | +1.5°C | Error |
| 온도 하한 경보 오프셋 | `tempAlarmLowOffsetC`  | -1.5°C | Error |
| 습도 상한 경보 오프셋 | `humidAlarmHighOffsetPct` | +10% | Warning |
| 습도 하한 경보 오프셋 | `humidAlarmLowOffsetPct`  | -10% | Warning |
| 알람 지속 확인 시간   | `alarmConfirmMs`       | 60000ms | — |

> `alarmConfirmMs`: 임계값 초과 상태가 이 시간 이상 지속되어야 알람을 발행한다.
> 일시적 측정 편차를 걸러낸다.

### 3.3 알람 발행 규칙

```
ClimateControlModule.tick() 내:

  tempError = currentTempC - activeTargetTempC

  if (tempError > settings.tempAlarmHighOffsetC):
      alarmHighTimer += tickDeltaMs
      if (alarmHighTimer >= settings.alarmConfirmMs):
          publishEvent(TempAlarmHigh, Severity::Error)
  else:
      alarmHighTimer = 0

  // 하한/습도도 동일 패턴
```

### 3.4 ClimateControlModule이 구독하는 Event

| 구독 Event | 목적 |
|---|---|
| `SensorRecovered` (Temp/Humi source) | 알람 타이머 리셋 |
| `SafeModeEntered` | Heater/Humi 강제 OFF |
| `BatchInactive` | 제어 루프 비활성화 |

---

## 4. TurningControl 정책

### 4.1 결정 — Phase 1: ON/OFF 전용 모듈

Phase 1에서 전란 장치는 **전용 전란 모듈(전기식 전란기)**을 사용한다.
`TurningControlModule`은 전란 모듈의 전원(Relay)을 켜고 끄는 것만 담당한다.

```
TurningControlModule
  → RelayModule(Turner) 제어 (ON = 전란 모듈 동작, OFF = 정지)
  → turningIntervalMin 주기 도달 시 Relay ON
  → turningDurationMin 경과 후 Relay OFF
```

### 4.2 전란 관련 AppSettings 필드

| 필드명 | 기본값 | 설명 |
|---|---|---|
| `turningDurationMin` | 3분 | 전란 1회 동작 시간 |

> `turningIntervalMin`은 Day Plan row에 per-day로 정의됨 (RuntimeState 경유).
> `turningDurationMin`은 전체 공통 설정이므로 AppSettings에 위치.

### 4.3 전란 Relay ID

ProductSpec의 Source ID 할당표에 Turner Relay 추가:

```cpp
// include/incubator/config/ProductIds.h — Relay_Turner 예시
static constexpr uint16_t Relay_Turner = 2003U;  // 전란 모듈 전원 Relay
```

### 4.4 Phase 2+ 검토: 스테퍼 직접 제어

스테퍼 모터 직접 제어 방식은 Phase 2 이후 `MotionModule` 기반으로 확장한다.
이때 고려해야 할 사항:

- 전란 1회 = 45° 회전 (스테퍼 마이크로스텝 설정에 따라 step 수 결정)
- 매 회 방향 교번 (홀수회 CW, 짝수회 CCW) — NVS에 마지막 방향 저장
- 전란 완료 판단: `MotionCompleted` Event 기준 (타임아웃 보조)
- 스텝 수 설정은 AppSettings에 추가 예정

### 4.5 TurningControlModule이 구독하는 Event

| 구독 Event | 목적 |
|---|---|
| `SafeModeEntered` | Turner 즉시 OFF |
| `BatchInactive` | 전란 루프 비활성화 |
| `LockdownStarted` | 전란 강제 OFF, 이후 스케줄 무시 |

---

## 5. AppSettings 완전 필드 정의

단일 진실은 `include/incubator/domain/model/AppSettings.h`이다. 아래는 필드·`defaults()` 개념 발췌이며, `isValid()` 등 전문은 저장소 헤더를 본다.

```cpp
// include/incubator/domain/model/AppSettings.h — 발췌
#pragma once
#include <cstdint>

namespace incubator::domain::model
{
    struct AppSettings
    {
        float    tempHysteresis;
        float    tempAlarmHighOffsetC;
        float    tempAlarmLowOffsetC;
        float    humidityHysteresis;
        float    humidAlarmHighOffsetPct;
        float    humidAlarmLowOffsetPct;
        uint32_t alarmConfirmMs;
        bool     alarmEnabled;
        uint16_t turningDurationMin;
        uint8_t  displayBrightness;
        bool     syncEnabled;
        uint32_t syncIntervalMs;

        static AppSettings defaults()
        {
            AppSettings s{};
            s.tempHysteresis          = 0.3f;
            s.tempAlarmHighOffsetC    = 1.5f;
            s.tempAlarmLowOffsetC     = -1.5f;
            s.humidityHysteresis      = 2.0f;
            s.humidAlarmHighOffsetPct = 10.0f;
            s.humidAlarmLowOffsetPct  = -10.0f;
            s.alarmConfirmMs          = 60000U;
            s.alarmEnabled            = true;
            s.turningDurationMin      = 3U;
            s.displayBrightness       = 80U;
            s.syncEnabled             = false;
            s.syncIntervalMs          = 300000U;
            return s;
        }

        bool isValid() const;  // 전문은 헤더
    };
}
```

---

## 6. RuntimeState 완전 필드 정의

단일 진실은 `include/incubator/domain/model/RuntimeState.h`이다.

```cpp
// include/incubator/domain/model/RuntimeState.h — 발췌
#pragma once
#include <cstdint>
#include <cstring>

namespace incubator::domain::model
{
    struct RuntimeState
    {
        float    currentTempC;
        float    currentHumidityPct;
        bool     tempSensorOk;
        bool     humiSensorOk;

        bool     heaterOn;
        bool     humidifierOn;
        bool     fanOn;
        bool     turnerOn;

        float    activeTargetTempC;
        float    activeTargetHumidityPct;
        bool     activeTurningEnabled;
        uint16_t activeTurningIntervalMin;
        bool     activeVentFanEnabled;

        uint16_t currentDay;
        uint16_t totalDays;
        bool     batchActive;
        bool     lockdownActive;

        uint32_t lastTurningEpoch;
        uint16_t nextTurningInMin;

        bool     tempAlarmActive;
        bool     humiAlarmActive;

        bool     safeMode;
        uint32_t uptimeMs;

        static RuntimeState zero()
        {
            RuntimeState s{};
            memset(&s, 0, sizeof(s));
            s.tempSensorOk = false;
            s.humiSensorOk = false;
            return s;
        }
    };
}
```

---

## 7. UiModel 완전 필드 정의

단일 진실은 `include/incubator/ui/UiModel.h`이다. 페이지 번호·필드 주석은 헤더와 `activePage`(0~4) 매핑을 본다.

```cpp
// include/incubator/ui/UiModel.h — 발췌
#pragma once
#include <cstdint>

namespace incubator::ui
{
    struct UiModel
    {
        float    displayTempC;
        float    displayHumidityPct;
        float    targetTempC;
        float    targetHumidityPct;
        uint16_t currentDay;
        uint16_t totalDays;
        bool     heaterOn;
        bool     humidifierOn;
        bool     fanOn;
        bool     turnerOn;
        bool     batchActive;
        bool     tempAlarm;
        bool     humiAlarm;
        bool     tempSensorFault;
        bool     humiSensorFault;

        uint16_t nextTurningInMin;
        uint8_t  progressPct;
        bool     lockdownActive;
        bool     turningEnabled;

        uint16_t editDay;
        float    editTargetTempC;
        float    editTargetHumidityPct;
        bool     editTurningEnabled;
        uint16_t editTurningIntervalMin;
        bool     editRowOverridden;

        uint32_t bootCount;
        uint32_t uptimeMs;
        bool     cloudConnected;
        char     ipAddress[16];

        uint8_t  activePage;
        bool     safeMode;
        bool     editMode;

        static UiModel zero();  // 전문은 헤더
    };
}
```

---

## 8. LocalPlanRepository 저장 전략

### 8.1 저장 위치 결정

| 데이터 | 저장소 | 키/경로 | NVS Namespace |
|---|---|---|---|
| `AppSettings` | NVS binary blob | `app_settings` | `incubator` |
| `IncubationBatch` | NVS binary blob | `batch` | `incubator` |
| `PlanSyncState` | NVS binary blob | `sync_state` | `incubator` |
| `IncubationPlanTable` | LittleFS JSON | `/plan/plan.json` | — |

### 8.2 NVS 키 이름 규칙

> NVS 키는 최대 15자. Namespace는 `incubator` (9자).

```
Namespace : "incubator"
Keys      : "app_settings"  (12자)
            "batch"         (5자)
            "sync_state"    (10자)
            "boot_cnt"      (8자, FwCore BootStateManager 호환)
```

### 8.3 LittleFS 경로 규칙

```
/plan/plan.json     ← 현재 활성 Plan
/plan/plan.bak      ← 이전 버전 백업 (savePlan 시 자동 rotate)
```

백업 규칙:
1. `savePlan()` 호출 시 기존 `plan.json` → `plan.bak`으로 rename
2. 새 `plan.json` 쓰기
3. 쓰기 실패 시 `plan.bak` → `plan.json` 복구 시도

### 8.4 JSON 스키마 (로컬/원격 공통 단일 표준)

ProductSpec §11.3의 JSON 구조를 그대로 따른다. 추가 규칙:

```json
{
  "tableVersion": 5,
  "lastUpdatedAtEpoch": 1746000000,
  "lockdownStartDay": 19,
  "species": "Chicken",
  "dayCount": 21,
  "rows": [
    {
      "day": 1,
      "targetTempC": 37.5,
      "targetHumidityPct": 55.0,
      "turningEnabled": true,
      "turningIntervalMin": 120,
      "ventFanEnabled": true,
      "userOverridden": false,
      "updatedAtEpoch": 0
    }
  ]
}
```

> ⚠️ 필드명은 이 표준으로 고정. ESP32 / Lambda / PHP / Blazor 모두 동일 필드명 사용.
> species 값: `"Chicken"` | `"Duck"` | `"Quail"` | `"Goose"` | `"Custom"`

### 8.5 LocalPlanRepository 초기화 흐름

```cpp
bool LocalPlanRepository::init()
{
    // 1. LittleFS mount
    if (!LittleFS.begin(true)) return false;

    // 2. /plan 디렉토리 없으면 생성
    if (!LittleFS.exists("/plan")) LittleFS.mkdir("/plan");

    return true;
}
```

---

## 9. 히스테리시스 및 PID 전환 전략

### 9.1 Phase 1: 히스테리시스 On-Off 제어

```
히터 ON  조건: currentTempC < (targetTempC - tempHysteresis)
히터 OFF 조건: currentTempC > (targetTempC + tempHysteresis)
```

`tempHysteresis` 기본값 0.3°C → 제어 대역: ±0.3°C (총 0.6°C 대역)

요구 정밀도(±0.2°C)보다 대역이 넓다.
→ 히스테리시스 0.1°C로 좁히면 정밀도는 올라가지만 SSR 스위칭 빈도 증가.
→ 실제 하드웨어 테스트 후 최적값 결정. AppSettings에서 조정 가능.

### 9.2 Phase 3: PID 제어 전환 계획

PID 도입 시 `ClimateControlModule`의 제어 전략만 교체, 인터페이스는 유지한다.

```cpp
// 전환 전 (Phase 1)
class OnOffClimateStrategy : public IClimateStrategy { ... };

// 전환 후 (Phase 3)
class PidClimateStrategy   : public IClimateStrategy { ... };

// ClimateControlModule은 IClimateStrategy*만 보유 → 전략 교체 가능
```

AppSettings에 향후 PID 파라미터 필드 추가 예정:

```cpp
// Phase 3 추가 예정 필드 (현재 미사용)
// float pidKp;    // 기본: 2.0
// float pidKi;    // 기본: 0.5
// float pidKd;    // 기본: 0.1
```

> **Phase 1 구현 원칙**: `IClimateStrategy` 인터페이스를 지금 정의해두고,
> `OnOffClimateStrategy`만 구현한다. PID 전환 시 코드 수정 최소화.

---

## 10. Product EventCode — FwCore 연동 방식

### 10.1 문제

FwCore의 `EventCode`는 `enum class EventCode : uint16_t`이고,
Product의 `ProductEventCode`도 `uint16_t` 기반이다.
두 타입을 혼합 사용할 때 타입 안전성 확보가 필요하다.

### 10.2 결정: 캐스팅 래퍼 함수 사용

```cpp
// include/incubator/config/ProductEventCodes.h (발췌 — 전문은 저장소 파일)
#pragma once
#include <fwcore/core/types/CoreTypes.h>

namespace incubator::config
{
    enum class ProductEventCode : uint16_t
    {
        BatchInactive       = 10000,
        BatchStarted        = 10001,
        BatchCompleted      = 10002,
        PlanGenerated       = 10100,
        PlanRowPatched      = 10101,
        PlanMissing         = 10102,
        PlanInvalid         = 10103,
        PlanVersionConflict = 10104,
        PlanSyncStarted     = 10200,
        PlanSyncSucceeded   = 10201,
        PlanSyncFailed      = 10202,
        LockdownStarted     = 10300,
        IncubationCompleted = 10301,
        TempAlarmHigh       = 10400,
        TempAlarmLow        = 10401,
        HumidAlarmHigh      = 10402,
        HumidAlarmLow       = 10403,
        TempAlarmCleared    = 10404,
        HumidAlarmCleared   = 10405,
    };

    inline fwcore::core::EventCode toFwCode(ProductEventCode c)
    {
        return static_cast<fwcore::core::EventCode>(
            static_cast<uint16_t>(c));
    }
}
```

### 10.3 사용 예시

```cpp
// Product Module 내부에서 Event 발행
m_eventBus.publish({
    .Code          = incubator::config::toFwCode(
                     incubator::config::ProductEventCode::TempAlarmHigh),
    .sourceId      = incubator::config::ProductIds::Module_ClimateControl,
    .severityLevel = fwcore::core::Severity::Error,
    .message       = "Temp alarm: above threshold",
    .timestamp     = m_clock.now(),
});
```

---

## 11. Display 하드웨어 및 라이브러리 결정

### 11.1 확정 사항

| 항목 | 결정값 |
|---|---|
| 디스플레이 모델 | ST7789, GMT020-02-7P 2.0인치 TFT SPI (**7핀**, 별도 BL 핀 없음) |
| 물리 해상도 | 240 × 320 (세로) |
| 펌웨어 사용 해상도 | **320 × 240** (가로 회전, landscape) |

```
### 11.3 St7789DisplayDevice 구현 계획

```
src/devices/St7789DisplayDevice.h/.cpp
    ← IDisplayDevice 구현
    ← beginFrame() / endFrame() / drawText() / drawProgress() 구현
```

**ProductSpec §5 및 구현은 `St7789DisplayDevice`를 사용한다.**

### 11.4 논리 해상도

```cpp
// 화면 좌표 기준
// Width  = 320px  (가로)
// Height = 240px  (세로)
// 원점(0,0) = 좌상단

namespace incubator::config::Pin
{
    static constexpr int DisplayWidth  = 320;
    static constexpr int DisplayHeight = 240;
}
```

---

## 12. Lockdown 구간 정책

### 12.1 정의

Lockdown = 부화 직전 전란을 중단해야 하는 구간.
전란 중단이 없으면 부화 시 병아리가 껍질 탈출에 실패할 수 있다.

### 12.2 종별 Lockdown 시작일

| 종 | 총 부화일 | Lockdown 시작일 | Lockdown 기간 |
|---|---|---|---|
| Chicken (닭) | 21일 | 19일차 | 마지막 3일 |
| Duck (오리) | 28일 | 26일차 | 마지막 3일 |
| Quail (메추라기) | 17일 | 15일차 | 마지막 3일 |
| Goose (거위) | 30일 | 28일차 | 마지막 3일 |
| Custom | 사용자 설정 | 사용자 설정 | 기본 3일 |

> **모든 종에서 기본 Lockdown 기간 = 마지막 3일** (totalDays - 2 일차부터)

### 12.3 IIncubationPresetPolicy에 lockdownStartDay 추가

```cpp
// include/incubator/domain/policy/IIncubationPresetPolicy.h
class IIncubationPresetPolicy
{
public:
    virtual ~IIncubationPresetPolicy() = default;

    virtual uint16_t getTotalDays()          const = 0;
    virtual uint16_t getLockdownStartDay()   const = 0;  // ← 추가
    virtual float    getTargetTempC(uint16_t day)         const = 0;
    virtual float    getTargetHumidityPct(uint16_t day)   const = 0;
    virtual bool     getTurningEnabled(uint16_t day)      const = 0;
    virtual uint16_t getTurningIntervalMin(uint16_t day)  const = 0;
    virtual bool     getVentFanEnabled(uint16_t day)      const = 0;
    virtual IncubationSpecies getSpecies()   const = 0;
};
```

### 12.4 IncubationPlanGenerator에서 Lockdown 행 자동 처리

```
day >= lockdownStartDay 인 row에 대해:
    turningEnabled     = false   (전란 비활성)
    targetHumidityPct += 5.0f   (Lockdown 구간 습도 상향, Preset 정책에 따름)
```

### 12.5 RuntimeState의 lockdownActive 갱신

`IncubationScheduleModule`이 매 Tick에 계산:

```cpp
runtime.lockdownActive = (runtime.currentDay >= plan.lockdownStartDay);
```

`lockdownActive == true`이면 `TurningControlModule`이 `LockdownStarted` Event를 수신하여
Turner Relay를 강제 OFF하고, 이후 스케줄 전란을 무시한다.


### 12.6 화면 레이아웃 (320×240 landscape)

┌──────────────────────── 320 ─────────────────────────────┐  y=0
│ Header HH:MM:SS           WiFi 아이콘 등           [PAGE] │  
├──────────────────────────────────────────────────────────┤
│                                                          │
│                                                          │
│                  Body                                    │  
│          각 페이지별 콘텐츠 렌더링                          │
│                                                          │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ Footer                                                   │  
│  HTR● HUM● FAN● TRN●  (상태 아이콘 행)                 D-1 │ 부화일
└──────────────────────────────────────────────────────────┘
핵심 설계 결정
계층 분리 완전 준수: MainUiRenderer는 UiModel만 읽고, UiController가 RuntimeState → UiModel 변환 담당. 렌더러 내부에서 RuntimeState 직접 접근 없음 확인.
Plan Edit UX 흐름:
버튼(짧게) → 편집 모드 진입
→ 회전: Day 탐색
→ 버튼(짧게): 첫 필드(Temp) 선택
→ 회전: 값 조정 (0.1°C / 1% step)
→ 버튼(짧게): 다음 필드 → Humi → Turning
→ 버튼(길게 1.5초): Save → applyPatch() → savePlan()

---

## 13. 미결 사항 및 향후 검토 항목

| ID | 항목 | 분류 | 담당 Phase | 상태 |
|---|---|---|---|---|
| OPEN-001 | PID 파라미터 최적값 결정 | HW 테스트 필요 | Phase 3 | 미결 |
| OPEN-002 | 스테퍼 모터 step/degree 교정값 | HW 사양 확정 후 | Phase 2+ | 미결 |
| OPEN-003 | 전란 방향 교번 구현 (NVS 방향 저장) | Phase 2 스테퍼 확장 시 | Phase 2 | 미결 |
| OPEN-004 | LittleFS 파티션 크기 | `partitions.csv` 설계 시 | Phase 1 | 미결 |
| OPEN-005 | 히스테리시스 최적값 | 실제 온도 안정성 테스트 후 | Phase 1 튜닝 | 미결 |
| OPEN-006 | `alarmConfirmMs` 60초 적정성 | 실운전 테스트 후 조정 | Phase 1 튜닝 | 미결 |
| OPEN-007 | Lockdown 구간 습도 상향폭 (+5%) | Preset 구현 시 종별 조정 | Phase 1 | 미결 |
| ~~OPEN-004~~ | ~~Lockdown 구간 정의~~ | — | — | **§12에서 확정** |
| ~~OPEN-005~~ | ~~TFT 라이브러리 선택~~ | — | — | **§11에서 확정** |

---

## 변경 이력

| 버전 | 날짜 | 변경 내용 |
|---|---|---|
| 1.0 | 2026-05-01 | 최초 작성 (DDU-01) |
| 1.1 | 2026-05-01 | §11 Display 결정 추가 (ST7789), §12 Lockdown 정책 확정 |
| 1.2 | 2026-05-03 | §11 TFT 사양(7핀·BL 없음) 및 LGFX 스니펫을 저장소(`LgfxConfig.h` 등)와 동기화 |
| 1.3 | 2026-05-03 | §10 `ProductEventCodes` 경로·네임스페이스(`incubator::config`) 및 발췌 내용을 저장소와 동기화 |
| 1.4 | 2026-05-03 | §1 I2C: `Esp32I2cBus`를 ESP-IDF 기반으로 정정, `DeviceRegistry` 초기화 순서를 소스와 동기화 |
| 1.5 | 2026-05-03 | §2.3 `Aht20Driver` 스니펫을 `include/incubator/devices/Aht20Driver.h` 및 네임스페이스와 동기화 |
| 1.6 | 2026-05-03 | §5~§7 `AppSettings`·`RuntimeState`·`UiModel` 경로·`incubator::` 네임스페이스 및 필드 발췌를 저장소 헤더와 동기화 |
| 1.7 | 2026-05-03 | 잔여 `src/domain/policy` 주석을 `include/incubator/domain/policy`로 정정 |
| 1.8 | 2026-05-03 | §8.4 Plan JSON 예시에 `lockdownStartDay` 필드 추가 (`LocalPlanRepository` 직렬화와 동기화) |
| 1.9 | 2026-05-03 | 상단 문서 `Version` 메타를 2.0으로 통일. 이후 주요 본문 수정 시 상단 `Version`과 본 변경 이렐을 함께 갱신한다 (`00_PLATFORM_BIBLE` §8 항 6). |
