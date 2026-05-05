# Incubator — Product Specification
> **Document ID**: Incubator-SPEC-001  
> **Version**: 2.0 | **Status**: Working Draft  
> **상위 플랫폼**: FwCore.Common (FWCORE-STD-001)  
> **대상 독자**: 펌웨어 개발자 (1인)  
> **목적**: Incubator 부화기 제품의 요건, 아키텍처, 구현 기준을 단일 문서로 정의한다.

---

## ⚠️ 개발 원칙 (읽기 전 확인)

```
이 제품은 FwCore.Common 위에 올라가는 Product Layer이다.
FwCore.Common의 계층 규칙, 명명 규칙, 금지 패턴을 항상 우선 준수한다.
이 문서가 FwCore.Common 문서와 충돌하면 FwCore.Common이 우선한다.
```

**FwCore.Common 참조 문서 우선순위**

| 순위 | 문서 | 참조 목적 |
|---|---|---|
| 1 | `00_PLATFORM_BIBLE.md` | 계층 규칙, 금지 패턴, 명명 규칙 |
| 2 | `00-1_FwCore_Naming_Rules.md` | 명명 규칙 상세 |
| 3 | `03_Interface_Reference.md` | 인터페이스 시그니처 |
| 4 | `05_Developer_Guide.md` | HAL/Device/Sink 구현 방법 |
| 5 | `06_Boilerplate.md` | main.cpp 및 조립 템플릿 |

---

## 목차

1. [제품 정의](#1-제품-정의)
2. [핵심 설계 원칙](#2-핵심-설계-원칙)
3. [기능 요건](#3-기능-요건)
4. [아키텍처](#4-아키텍처)
5. [프로젝트 구조](#5-프로젝트-구조)
6. [Source ID 할당표](#6-source-id-할당표)
7. [데이터 모델](#7-데이터-모델)
8. [부화 정책 (Preset)](#8-부화-정책-preset)
9. [Day Plan 테이블](#9-day-plan-테이블)
10. [제어 모듈 설계](#10-제어-모듈-설계)
11. [저장소 설계](#11-저장소-설계)
12. [동기화 설계](#12-동기화-설계)
13. [로컬 UI 설계](#13-로컬-ui-설계)
14. [이벤트 정의](#14-이벤트-정의)
15. [부트 시퀀스](#15-부트-시퀀스)
16. [구현 순서](#16-구현-순서)
17. [개발 단계 (Phase)](#17-개발-단계-phase)
18. [미결 사항 (Open Issues)](#18-미결-사항-open-issues)

---

## 1. 제품 정의

### 1.1 제품 개요

Incubator는 FwCore.Common 기반의 **조류 부화기 제어 펌웨어**이다.

| 항목 | 내용 |
|---|---|
| 타겟 MCU | ESP32-S3 |
| 프레임워크 | PlatformIO + ESP-IDF + Arduino as component |
| C++ 표준 | C++17 |
| 로컬 TFT | ST7789 240×320, GMT020-02-7P 2.0인치 SPI (7핀, 별도 BL 핀 없음) |
| 클라우드 | AWS IoT Core / Lambda |
| 원격 UI | Blazor WebAssembly (PHP 호스팅) |

### 1.2 제품이 하는 일

```
부화 정책(Preset) → 일자별 계획 테이블(Day Plan) 자동 생성
                                    ↓
                        사용자가 Row 단위 보정 가능
                                    ↓
                실제 운전은 항상 최종 테이블 기준으로 동작
```

**단일 진실 원천(Single Source of Truth)**: Day Plan Table

---

## 2. 핵심 설계 원칙

### 2.1 테이블이 유일한 제어 기준이다

- Preset/Profile은 **초기값 생성기**일 뿐이다.
- 사용자는 생성된 테이블의 **row를 개별 보정**할 수 있다.
- 실제 디바이스 제어는 항상 최종 테이블 row를 기준으로 한다.
- `UserOverridden = true`인 row는 테이블 재생성 시 보존 여부를 선택할 수 있다.

### 2.2 데이터와 정책을 분리한다

| 계층 | 책임 | 절대 금지 |
|---|---|---|
| `AppSettings` | 운영 설정 저장 | 계산 로직 |
| `RuntimeState` | 현재 상태 스냅샷 | 목표값 계산 함수 |
| `IncubationPlanTable` | Day Plan 저장 | 제어 판단 |
| `ProductPolicies` | 정책, 부화일 계산, 목표값 계산 | 디바이스 직접 제어 |
| Module | 테이블 읽기 + 제어만 | Recovery 판단, reboot |

### 2.3 Day 기준과 Time 기준을 분리한다

- **Day Plan**: 부화 n일차에 대한 목표값 (정적)
- **Runtime Control**: 현재 시점(epoch, incubation day)에 따라 해당 Day Plan 적용 (동적)

### 2.4 로컬과 원격은 동일 데이터 모델을 사용한다

ESP32, AWS IoT Lambda, PHP API, Blazor WASM이 **동일 JSON 구조**를 사용한다. 필드명 불일치는 나중에 반드시 버그를 유발한다.

### 2.5 FwCore.Common 계층 규칙 준수

```
✅ Product → API (AddXxx) → Module → Core → HAL → Hardware
❌ Product에서 Device 직접 제어
❌ Module에서 Recovery 판단 또는 reboot
❌ Device에서 EventBus 접근
```

---

## 3. 기능 요건

### 3.1 필수 기능 1단계

| ID | 기능 | 설명 |
|---|---|---|
| REQ-F-001 | 부화 일수 기준 제어 | Epoch 기준 현재 일수 자동 계산 |
| REQ-F-002 | 일자별 목표값 | 온도/습도/전란/팬 per-day 설정 |
| REQ-F-003 | 조류별 Preset | Chicken, Duck, Quail, Goose, Custom |
| REQ-F-004 | 테이블 자동 생성 | Preset → Day Plan Table |
| REQ-F-005 | Row 개별 보정 | 특정 일자 항목 수정, UserOverridden 관리 |
| REQ-F-006 | 테이블 기준 운전 | 수정 완료된 테이블로 실제 제어 |
| REQ-F-007 | 로컬 UI | TFT 화면, 로터리 엔코더 입력 |
| REQ-F-008 | 이상 감지 알람 | 로컬 부저 + 원격 알람(KakaoTalk 등) |
| REQ-F-009 | 안전 복구 | FwCore RecoveryEngine 기반 자동 복구 |

### 3.2 필수 기능 2단계

| ID | 기능 | Phase |
|---|---|---|
| REQ-F-101 | BLE 프로비저닝 | Phase 2 |
| REQ-F-102 | AWS IoT 원격 설정/모니터링 | Phase 2 |
| REQ-F-103 | 로컬/원격 데이터 동기화 | Phase 2 |
| REQ-F-104 | Blazor WASM 원격 UI | Phase 2 |
| REQ-F-105 | OTA 펌웨어 업데이트 | Phase 3 |

### 3.3 비기능 요건

| 항목 | 기준 |
|---|---|
| Tick 주기 | < 50ms (Watchdog 5초 기준) |
| 온도 제어 정밀도 | ±0.2°C 이내 (히스테리시스 포함) |
| 전원 재투입 복구 | 부팅 후 이전 상태 자동 복원 |
| 데이터 영속성 | NVS + LittleFS, 재부팅 후 유지 |
| 코드 언어 | C++17, 동적 할당 Tick 내 금지 |

---

## 4. 아키텍처

### 4.1 계층 다이어그램

```
┌────────────────────────────────────────────────────────┐
│                   PRODUCT LAYER                        │
│  ProductBootstrap  ProductPolicies  ProductIds         │
│  RegisterServices  UiModel  MainUiRenderer             │
└────────────────────────┬───────────────────────────────┘
                         │ AddXxx(...) + build()
┌────────────────────────▼───────────────────────────────┐
│          PUBLIC API / COMPOSITION LAYER                │
│          (FwCore.Common: ServiceBuilder)               │
└───────┬────────────────────────────┬───────────────────┘
        │                            │
┌───────▼──────────────┐  ┌──────────▼──────────────────┐
│   PRODUCT MODULES    │  │   INFRASTRUCTURE SERVICE    │
│                      │  │                             │
│  IncubationSchedule  │  │   CloudClient (AWS IoT)     │
│    Module            │  │   ProvisioningService (BLE) │
│  ClimateControl      │  │   OtaService                │
│    Module            │  │                             │
│  TurningControl      │  │ (모두 FwCore 상태머신 기반)  │
│    Module            │  └──────────────┬──────────────┘
└───────┬──────────────┘                 │
        │  publishEvent()                │
        └──────────────────┬─────────────┘
                           │
┌──────────────────────────▼─────────────────────────────┐
│               CORE SYSTEM LAYER (FwCore.Common)        │
│  EventBus  RecoveryEngine  TraceLogger  AlarmPublisher │
│  SystemKernel  HealthMonitor  SafeModeController       │
└──────────────────────────┬─────────────────────────────┘
                           │
┌──────────────────────────▼─────────────────────────────┐
│                      HAL LAYER                         │
│    Esp32Clock  Esp32Watchdog  Esp32NvsStorage          │
└──────────────────────────┬─────────────────────────────┘
                           │
                     [ Hardware ]
              Aht20  Relay(SSR)  (스테퍼 Phase 2+)  TFT  Encoder
```

### 4.2 Product 전용 데이터 흐름

```
[IIncubationPresetPolicy]   ← ChickenPresetPolicy 등
          ↓
[IncubationPlanGenerator]   ← Preset → Table 자동 생성
          ↓
[IncubationPlanTable]       ← 단일 진실 원천 (Day 0~N)
          ↓ (로컬 편집 / 원격 편집)
[IIncubationPlanRepository] ← NVS + LittleFS / AWS IoT
          ↓
[PlanSyncService]           ← 로컬 ↔ 원격 동기화
          ↓
[IncubationScheduleModule]  ← 현재 day 계산 → 목표값 → RuntimeState 갱신
          ↓
[ClimateControlModule]      ← 센서값 vs 목표값 → Relay/Fan 제어
[TurningControlModule]      ← TurningEnabled → MotionModule 제어
```

---

## 5. 프로젝트 구조

헤더는 `include/incubator/`에, 구현은 주로 `src/`에 둔다 (PlatformIO + ESP-IDF `src` 컴포넌트).

```
Incubator/
├── platformio.ini
├── sdkconfig.defaults
├── partitions.csv
│
├── components/
│   ├── arduino/                     ← 별도 프로젝트에 공유됨
│   └── fwcore_wrapper/              ← FwCore.Common → ESP-IDF 컴포넌트 래퍼
│
├── third_party/
│   └── FwCore.Common/               ← git submodule (읽기 전용)
│
├── include/incubator/               ← Product 공개 헤더
│   ├── config/
│   │   ├── ProductIds.h
│   │   ├── PinConfig.h
│   │   └── ProductEventCodes.h
│   ├── hal/
│   │   ├── Esp32Clock.h
│   │   ├── Esp32Watchdog.h
│   │   ├── Esp32NvsStorage.h
│   │   └── Esp32I2cBus.h
│   ├── devices/
│   │   ├── DeviceRegistry.h
│   │   ├── Aht20Driver.h
│   │   ├── Aht20SensorDevice.h      ← Aht20TempDevice / Aht20HumiDevice 선언
│   │   ├── GpioRelayDevice.h
│   │   ├── PwmFanDevice.h
│   │   ├── St7789DisplayDevice.h
│   │   └── Ec11InputDevice.h
│   ├── observability/
│   │   ├── SerialTraceSink.h
│   │   └── BuzzerAlarmSink.h
│   ├── domain/
│   │   ├── model/                   ← IncubationSpecies, Batch, Plan…, AppSettings, RuntimeState …
│   │   ├── policy/                  ← Preset 정책, IncubationPlanGenerator, IncubationDayResolver …
│   │   └── repository/
│   │       ├── IIncubationPlanRepository.h
│   │       └── LocalPlanRepository.h
│   ├── modules/
│   │   ├── IncubationScheduleModule.h
│   │   ├── ClimateControlModule.h
│   │   └── TurningControlModule.h
│   ├── composition/
│   │   ├── RegisterServices.h
│   │   └── AppState.h
│   └── ui/
│       ├── UiModel.h
│       ├── UiController.h
│       └── MainUiRenderer.h
│
└── src/
    ├── main.cpp
    ├── hal/
    │   ├── Esp32NvsStorage.cpp
    │   └── Esp32I2cBus.cpp
    ├── devices/
    │   ├── DeviceRegistry.cpp
    │   ├── Aht20Driver.cpp
    │   ├── Aht20TempHumiDevices.cpp
    │   ├── GpioRelayDevice.cpp
    │   ├── PwmFanDevice.cpp
    │   ├── St7789DisplayDevice.cpp
    │   └── Ec11InputDevice.cpp
    ├── domain/
    │   ├── policy/*.cpp
    │   └── repository/LocalPlanRepository.cpp
    ├── modules/
    │   ├── IncubationScheduleModule.cpp
    │   ├── ClimateControlModule.cpp
    │   └── TurningControlModule.cpp
    ├── composition/
    │   └── RegisterServices.cpp
    └── ui/
        ├── UiController.cpp
        └── MainUiRenderer.cpp
```

**Phase 2+ (저장소에 아직 없을 수 있음):** `RemotePlanRepository`, `PlanSyncService`, `IMotionDevice` 스테퍼 등.

### 5.1 폴더별 책임 요약

| 폴더 | 책임 | FwCore 인터페이스 구현 |
|---|---|---|
| `include/incubator/` | 제품 헤더·선언 (config, hal, devices, domain, …) | 인터페이스 구현체 선언 |
| `src/` | 제품 `.cpp` 구현 | `include/incubator`와 1:1 대응 |
| `include/incubator/hal/` | MCU 종속 HAL 선언 | `IClock`, `IWatchdog`, `IStorage`, `II2cBus` 등 |
| `include/incubator/devices/` | 디바이스 캡슐화 선언 | `ISensorDevice`, `IRelayDevice` 등 |
| `include/incubator/observability/` | 출력 채널 | `ITraceSink`, `IAlarmSink` |
| `include/incubator/domain/model/` | 데이터 구조 | — (순수 데이터) |
| `include/incubator/domain/policy/` | 부화 정책, 계산 | — (비즈니스 로직) |
| `include/incubator/domain/repository/` | 저장/로드 | `IIncubationPlanRepository` |
| `include/incubator/modules/` | 상태 해석, Event 발행 | `IModule` |
| `include/incubator/composition/` | 서비스 선언 조립 | `AddXxx()` 호출 |
| `include/incubator/ui/` | 화면 렌더링 | `IDisplayRenderer` |

---

## 6. Source ID 할당표

> **규칙**: FwCore 예약 구간(1~99)은 절대 사용 금지. 한번 할당된 ID 재사용 금지.

```cpp
// include/incubator/config/ProductIds.h (발췌 — 전문은 저장소 파일)
#pragma once
#include <cstdint>

namespace incubator::config::ProductIds
{
    static constexpr uint16_t Sensor_Temp      = 1001U;
    static constexpr uint16_t Sensor_Humidity  = 1002U;
    static constexpr uint16_t Relay_Heater     = 2001U;
    static constexpr uint16_t Relay_Humidifier = 2002U;
    static constexpr uint16_t Relay_Turner     = 2003U;
    static constexpr uint16_t Motion_Turner    = 3001U;  // Phase 2+
    static constexpr uint16_t Display_Main     = 4001U;
    static constexpr uint16_t Input_Encoder    = 4101U;
    static constexpr uint16_t Fan_Vent         = 5001U;
    static constexpr uint16_t Infra_Provisioning = 6001U;
    static constexpr uint16_t Infra_Cloud        = 6101U;
    static constexpr uint16_t Infra_Ota          = 6301U;
    static constexpr uint16_t Module_IncubSchedule   = 7001U;
    static constexpr uint16_t Module_ClimateControl  = 7002U;
    static constexpr uint16_t Module_TurningControl  = 7003U;
    static constexpr uint16_t Module_PlanSync        = 7004U;  // Phase 2+
}
```

---

## 7. 데이터 모델

> **원칙**: 모델 구조체는 데이터만 보유한다. 계산 함수는 `domain/policy/`에 둔다.

> **경로**: 구현 헤더는 `include/incubator/domain/model/`·`include/incubator/domain/policy/` 등 `include/incubator/` 아래에 있다. 필드 전문·`AppSettings`/`RuntimeState` 완전 정의는 `docs/22_Incubator_DetailDesign.md` §5~§6 및 동일 이름 헤더가 단일 진실이다.

### 7.1 IncubationSpecies

```cpp
// include/incubator/domain/model/IncubationSpecies.h
namespace incubator::domain::model
{
    enum class IncubationSpecies : uint8_t
    {
        Chicken = 0,
        Duck    = 1,
        Quail   = 2,
        Goose   = 3,
        Custom  = 4
    };
}
```

### 7.2 IncubationPlanRow (단일 진실 원천의 최소 단위)

`IncubationPlanRowPatch` 등은 헤더 전문을 본다.

```cpp
// include/incubator/domain/model/IncubationPlanRow.h
namespace incubator::domain::model
{
    struct IncubationPlanRow
    {
        uint16_t day;
        float    targetTempC;
        float    targetHumidityPct;
        bool     turningEnabled;
        uint16_t turningIntervalMin;
        bool     ventFanEnabled;
        bool     userOverridden;
        uint32_t updatedAtEpoch;
    };
}
```

### 7.3 IncubationPlanTable

`findRow` / `isValid` / `applyPatch` 등은 헤더 전문을 본다.

```cpp
// include/incubator/domain/model/IncubationPlanTable.h
namespace incubator::domain::model
{
    static constexpr uint16_t MaxPlanDays = 40U;

    struct IncubationPlanTable
    {
        IncubationPlanRow rows[MaxPlanDays];
        uint16_t          dayCount;
        uint16_t          lockdownStartDay;
        uint32_t          tableVersion;
        uint32_t          lastUpdatedAtEpoch;
        IncubationSpecies species;
        bool              dirty;
    };
}
```

### 7.4 IncubationBatch

`makeEmpty` / `makeNew` 등은 헤더 전문을 본다.

```cpp
// include/incubator/domain/model/IncubationBatch.h
namespace incubator::domain::model
{
    struct IncubationBatch
    {
        bool              active;
        IncubationSpecies species;
        uint32_t          startEpochSeconds;
        uint32_t          version;
        char              label[32];
    };
}
```

### 7.5 AppSettings

필드 전체·`defaults()`·`isValid()`·NVS 키는 **`docs/22_Incubator_DetailDesign.md` §5** 및 `include/incubator/domain/model/AppSettings.h`를 본다. (이 절의 이전 축약 스니펫은 폐기한다.)

### 7.6 RuntimeState

필드 전체·`zero()`·갱신 책임 모듈 매핑은 **`docs/22_Incubator_DetailDesign.md` §6** 및 `include/incubator/domain/model/RuntimeState.h`를 본다. (이 절의 이전 축약 스니펫은 폐기한다.)

### 7.7 PlanSyncState

Phase 1에서는 미사용·NVS에만 저장될 수 있다. 전문은 `include/incubator/domain/model/PlanSyncState.h`를 본다.

```cpp
// include/incubator/domain/model/PlanSyncState.h
namespace incubator::domain::model
{
    struct PlanSyncState
    {
        uint32_t lastPulledAtEpoch;
        uint32_t lastPushedAtEpoch;
        uint32_t lastKnownServerVersion;
        bool     hasConflict;
        bool     pushPending;

        static PlanSyncState zero();
    };
}
```

---

## 8. 부화 정책 (Preset)

### 8.1 인터페이스

```cpp
// include/incubator/domain/policy/IIncubationPresetPolicy.h
namespace incubator::domain::policy
{
    class IIncubationPresetPolicy
    {
    public:
        virtual ~IIncubationPresetPolicy() = default;
        virtual incubator::domain::model::IncubationSpecies
                         getSpecies()            const = 0;
        virtual uint16_t getTotalDays()          const = 0;
        virtual uint16_t getLockdownStartDay()   const = 0;
        virtual float    getTargetTempC(uint16_t day)       const = 0;
        virtual float    getTargetHumidityPct(uint16_t day) const = 0;
        virtual bool     isTurningEnabled(uint16_t day)     const = 0;
        virtual uint16_t getTurningIntervalMin(uint16_t day) const = 0;
        virtual bool     isVentFanEnabled(uint16_t day)     const = 0;
    };
}
```

### 8.2 조류별 Preset 표준값

| 조류 | 총 일수 | Lockdown 시작 | 온도 (초기/후기) | 습도 (초기/중기/후기) |
|---|---|---|---|---|
| Chicken | 21 | 19 | 37.5 / 37.2 | 55 / 53 / 65 |
| Duck | 28 | 25 | 37.5 / 37.2 | 55 / 55 / 70 |
| Quail | 18 | 15 | 37.5 / 37.2 | 45 / 45 / 65 |
| Goose | 30 | 27 | 37.5 / 37.2 | 55 / 55 / 70 |
| Custom | 사용자 정의 | 사용자 정의 | 사용자 정의 | 사용자 정의 |

### 8.3 Chicken 예시 구현

```cpp
// include/incubator/domain/policy/ChickenPresetPolicy.h
namespace incubator::domain::policy
{
class ChickenPresetPolicy final : public IIncubationPresetPolicy
{
public:
    incubator::domain::model::IncubationSpecies
    getSpecies() const override { return incubator::domain::model::IncubationSpecies::Chicken; }
    uint16_t getTotalDays() const override { return 21U; }
    uint16_t getLockdownStartDay() const override { return 19U; }

    float getTargetTempC(uint16_t day) const override
    {
        return (day <= 18) ? 37.5f : 37.2f;
    }
    float getTargetHumidityPct(uint16_t day) const override
    {
        if (day <= 7)  return 55.0f;
        if (day <= 18) return 53.0f;
        return 65.0f;
    }
    bool isTurningEnabled(uint16_t day) const override
    {
        return day < getLockdownStartDay();
    }
    uint16_t getTurningIntervalMin(uint16_t /*day*/) const override { return 120U; }
    bool isVentFanEnabled(uint16_t /*day*/) const override { return true; }
};
}
```

### 8.4 Policy Factory

```cpp
// include/incubator/domain/policy/IncubationPolicyFactory.h
namespace incubator::domain::policy
{
    class IncubationPolicyFactory
    {
    public:
        static const IIncubationPresetPolicy& resolve(
            incubator::domain::model::IncubationSpecies species);
        static CustomPresetPolicy& customPolicy();

    private:
        static ChickenPresetPolicy s_chicken;
        static DuckPresetPolicy    s_duck;
        static QuailPresetPolicy   s_quail;
        static GoosePresetPolicy   s_goose;
        static CustomPresetPolicy  s_custom;
    };
}
```

---

## 9. Day Plan 테이블

### 9.1 테이블 자동 생성

```cpp
// include/incubator/domain/policy/IncubationPlanGenerator.h
namespace incubator::domain::policy
{
    enum class RegenerateMode : uint8_t
    {
        FullRegenerate,
        PreserveOverrides,
    };

    class IncubationPlanGenerator
    {
    public:
        bool generate(
            const incubator::domain::model::IncubationBatch& batch,
            const IIncubationPresetPolicy& preset,
            incubator::domain::model::IncubationPlanTable& outTable,
            uint32_t nowEpochSeconds) const;

        bool regenerate(
            const IIncubationPresetPolicy& preset,
            incubator::domain::model::IncubationPlanTable& inOutTable,
            uint32_t nowEpochSeconds,
            RegenerateMode mode = RegenerateMode::PreserveOverrides) const;
    };
}
```

**생성 규칙:**

1. `preset.getTotalDays()`만큼 row 생성
2. day 1 ~ N 까지 각 row에 목표값 입력
3. `userOverridden = false`로 초기화
4. `lockdownStartDay` ← `preset.getLockdownStartDay()`
5. `tableVersion`, `lastUpdatedAtEpoch` 갱신

### 9.2 테이블 재생성 모드

| 모드 | 동작 | 권장 사용 시점 |
|---|---|---|
| Full Regenerate | 모든 row를 preset으로 덮어씀 | 종(species) 변경 시 |
| Preserve Overrides | `userOverridden=true` row는 보존, 나머지만 갱신 | 단순 정책 파라미터 조정 시 |

**기본값: Preserve Overrides**

### 9.3 Row 보정 (Patch)

필드 정의는 **`include/incubator/domain/model/IncubationPlanRow.h`**의 `IncubationPlanRowPatch`를 본다(`patchVentFan` 등 포함).

**보정 적용 후 반드시:**
- `row.userOverridden = true`
- `table.tableVersion++`
- `table.dirty = true`

---

## 10. 제어 모듈 설계

> **공통 원칙**: 모든 Module은 FwCore `IModule`을 구현한다. Recovery 판단 금지. Event 발행만.

### 10.1 IncubationScheduleModule

**책임**: 현재 부화 일수 계산 → 해당 Day Plan row 조회 → `RuntimeState` 목표값 갱신

```
Tick 시:
  1. batch.active 확인 → false면 BatchInactive event 발행
  2. IncubationDayResolver.resolveDay(batch.start, now, totalDays)
  3. planTable.rows[day-1] 조회 → 없으면 PlanMissing event 발행
  4. RuntimeState.activeTargetTempC 갱신
  5. RuntimeState.activeTurningEnabled 갱신
  6. RuntimeState.currentDay 갱신
```

**하면 안 되는 것**: relay 직접 ON/OFF, 서버 통신 직접 처리

### 10.2 ClimateControlModule

**책임**: RuntimeState의 현재값 vs 목표값 비교 → Heater/Humidifier/Fan Relay 제어

```cpp
// 히스테리시스 기반 제어 로직
// Tick 내에서 non-blocking으로 실행
void onTick()
{
    // 히터 제어
    if (runtime.currentTempC < runtime.activeTargetTempC - settings.tempHysteresis)
        heaterRelay.set(true);
    else if (runtime.currentTempC > runtime.activeTargetTempC + settings.tempHysteresis)
        heaterRelay.set(false);

    // 가습기 제어
    if (runtime.currentHumidityPct < runtime.activeTargetHumidityPct - settings.humidityHysteresis)
        humidifierRelay.set(true);
    else if (runtime.currentHumidityPct > runtime.activeTargetHumidityPct + settings.humidityHysteresis)
        humidifierRelay.set(false);
}
```

**Fan 제어 정책 (Phase 1 rule-based):**

- 히터 또는 가습기 동작 중: 보조 팬 ON
- 두 장치 모두 OFF: 팬 OFF
- PID 확장은 Phase 2 이후 검토

### 10.3 TurningControlModule

**책임**: `RuntimeState.activeTurningEnabled` 및 간격에 따라 **전란 액추에이터**를 제어한다.

**Phase 1 (현재 저장소):** 전란 모듈 전원은 **SSR 릴레이(`IRelayDevice`)**로 on/off 한다.

**Phase 2+ (예정):** 스테퍼 등 `IMotionDevice`·`MotionModule` 기반 직접 구동으로 확장할 수 있다.

```
Tick 시 (Phase 1 기준):
  1. runtime.activeTurningEnabled == false 또는 lockdown → 전란 릴레이 OFF
  2. true이고 간격 만족 시 → 전란 릴레이 ON (동작 시간은 정책/AppSettings 따름)
```

### 10.4 현재 일수 계산

```cpp
// include/incubator/domain/policy/IncubationDayResolver.h
namespace incubator::domain::policy
{
    class IncubationDayResolver
    {
    public:
        uint16_t resolveDay(uint32_t startEpochSeconds,
                            uint32_t nowEpochSeconds,
                            uint16_t totalDays) const;

        bool isCompleted(uint32_t startEpochSeconds,
                         uint32_t nowEpochSeconds,
                         uint16_t totalDays) const;

        bool isLockdown(uint32_t startEpochSeconds,
                        uint32_t nowEpochSeconds,
                        uint16_t totalDays,
                        uint16_t lockdownStartDay) const;
    };
}
```

---

## 11. 저장소 설계

### 11.1 저장 전략

| 데이터 | 저장 위치 | 이유 |
|---|---|---|
| AppSettings | NVS binary | 소용량, 빠른 접근 |
| IncubationBatch | NVS binary | 소용량, 자주 읽기 |
| IncubationPlanTable | LittleFS JSON | 디버깅 편의, 원격과 JSON 공유 |
| PlanSyncState | NVS binary | 소용량, 빠른 접근 |

### 11.2 Repository 인터페이스

```cpp
// include/incubator/domain/repository/IIncubationPlanRepository.h
namespace incubator::domain::repository
{
    class IIncubationPlanRepository
    {
    public:
        virtual ~IIncubationPlanRepository() = default;

        virtual bool init() = 0;

        virtual bool loadSettings(
            incubator::domain::model::AppSettings& out) = 0;
        virtual bool saveSettings(
            const incubator::domain::model::AppSettings& settings) = 0;

        virtual bool loadBatch(
            incubator::domain::model::IncubationBatch& out) = 0;
        virtual bool saveBatch(
            const incubator::domain::model::IncubationBatch& batch) = 0;

        virtual bool loadPlan(
            incubator::domain::model::IncubationPlanTable& out) = 0;
        virtual bool savePlan(
            const incubator::domain::model::IncubationPlanTable& plan) = 0;

        virtual bool loadSyncState(
            incubator::domain::model::PlanSyncState& out) = 0;
        virtual bool saveSyncState(
            const incubator::domain::model::PlanSyncState& state) = 0;

        virtual bool clearAll() = 0;
    };
}
```

### 11.3 JSON 필드 표준 (로컬/원격 공통)

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

> ⚠️ 필드명은 이 표준에서 고정한다. ESP32 / Lambda / PHP / Blazor 모두 동일 필드명 사용.

---

## 12. 동기화 설계

### 12.1 버전 관리 필수 필드

- `IncubationBatch.version`
- `IncubationPlanTable.tableVersion`
- `lastUpdatedAtEpoch`

### 12.2 충돌 처리 정책

| 상황 | 처리 |
|---|---|
| 로컬 > 서버 버전 | 로컬 → 서버 Push |
| 서버 > 로컬 버전 | 서버 → 로컬 Pull |
| 버전 동일 | 동기화 불필요 |
| 타임스탬프 충돌 | `PlanVersionConflict` Event 발행, 사용자 확인 대기 |

### 12.3 PlanSyncService

- FwCore `IModule` 구현 (Tick 기반 non-blocking)
- 연결 상태는 `CloudClient`에서 읽음
- 동기화 결과를 Event로 발행
- blocking retry loop 절대 금지

---

## 13. 로컬 UI 설계

### 13.1 화면 구성 (TFT 기준)

| Page | 내용 | 입력 |
|---|---|---|
| PAGE 1 (Main) | Header: 시각/WiFi/배치번호. Body: 온도·습도 크게. Footer: Heater/Humi/Turner/Fan 상태 아이콘 + 현재 Day | 엔코더 회전: 페이지 이동 |
| PAGE 2 (Status) | 오늘 목표값(온도/습도/전란 여부), 다음 전란까지 남은 시간, 부화 진행률 | — |
| PAGE 3 (Manual) | Heater / Humidifier / Turner / Fan 수동 ON/OFF 테스트 | 엔코더 선택 + 버튼 |
| PAGE 4 (Plan Edit) | Day 선택 → 항목 선택(Temp/Humi/Turning) → 값 조정 → Save | 엔코더 |
| PAGE 5 (System) | 배치 정보, 동기화 상태, 부팅 횟수, IP 주소 | — |

### 13.2 편집 UX

```
Plan Edit (PAGE 4)
 → Day 선택 (1~N, 엔코더 회전)
 → 항목 선택 (Temp / Humidity / Turning, 엔코더 버튼)
 → 값 조정 (온도 0.1°C step, 습도 1% step, 전란 toggle)
 → Save (버튼 길게 누름)
 → UserOverridden = true 자동 설정
```

### 13.3 SafeMode 표시

```cpp
void MainUiRenderer::render(IDisplayDevice& device)
{
    device.beginFrame();
    device.Clear();
    if (g_safeMode.isSafeMode()) {
        device.drawText(10, 10, "!! SAFE MODE !!");
        device.drawText(10, 30, "Check sensors");
    } else {
        renderNormalUi(device);
    }
    device.endFrame();
}
```

---

## 14. 이벤트 정의

> Product 전용 EventCode는 FwCore CoreTypes.h의 예약 범위를 침범하지 않도록 **10000번 이상**을 사용한다.

```cpp
// include/incubator/config/ProductEventCodes.h
// FwCore EventCode는 최대 9999까지 예약됨 → Product는 10000+
namespace incubator::config
{
    enum class ProductEventCode : uint16_t
    {
        // Incubation Domain (10000~10099)
        BatchInactive       = 10000,  // 활성 배치 없음
        BatchStarted        = 10001,  // 배치 시작
        BatchCompleted      = 10002,  // 부화 완료

        // Plan (10100~10199)
        PlanGenerated       = 10100,  // 테이블 자동 생성 완료
        PlanRowPatched       = 10101,  // Row 보정 완료
        PlanMissing         = 10102,  // 현재 day row 없음 (Error)
        PlanInvalid         = 10103,  // 테이블 유효성 실패 (Error)
        PlanVersionConflict = 10104,  // 버전 충돌 (Warning)

        // Sync (10200~10299) — Phase 2+
        PlanSyncStarted     = 10200,
        PlanSyncSucceeded   = 10201,
        PlanSyncFailed      = 10202,  // Error

        // Climate / Schedule (10300~10399)
        LockdownStarted     = 10300,  // 전란 금지 구간 진입
        IncubationCompleted = 10301,  // 예정 부화일 완료

        // Alarm (10400~10499)
        TempAlarmHigh       = 10400,  // 온도 상한 초과 (Critical)
        TempAlarmLow        = 10401,  // 온도 하한 미달 (Critical)
        HumidAlarmHigh      = 10402,  // 습도 상한 초과 (Warning)
        HumidAlarmLow       = 10403,  // 습도 하한 미달 (Warning)
        TempAlarmCleared    = 10404,
        HumidAlarmCleared   = 10405,
    };
}
```

---

## 15. 부트 시퀀스

```
Power On / Reset
  │
  ├─ [Platform] HAL 초기화 (Esp32Clock, Esp32Watchdog; I2C는 `Esp32I2cBus`, TFT는 SPI — 순서는 `main.cpp`·`DeviceRegistry` 기준)
  │
  ├─ [Product] Core 객체 생성 (g_eventBus, g_recovery, g_kernel ...)
  │
  ├─ [Product] Sink 등록 (AddTraceSink: serial, AddAlarmSink: buzzer)
  │
  ├─ [Product] Device 초기화 (DeviceRegistry::init())
  │
  ├─ [Product] 저장소 로드
  │             AppSettings, IncubationBatch, IncubationPlanTable
  │
  ├─ [Product] Plan 유효성 검사
  │             유효 → 정상 진행
  │             Batch 있고 Plan 없음 → Preset으로 자동 생성
  │             Plan 깨짐 → PlanInvalid Event + Preset으로 안전 재생성
  │
  ├─ [Product] RegisterServices() → AddXxx() 선언형 등록
  │
  ├─ [API]     services.build() → Frozen = true
  │
  ├─ [Product] g_kernel.registerModule(g_health)
  │
  ├─ [Core]    g_kernel.setup()
  │             → BootStateManager 이력 로드/평가
  │             → BootNormal / BootLoopDetected Event 발행
  │
  └─ [Runtime] loop() → g_kernel.tick() 반복
```

**Fallback 규칙:**

| 상황 | 처리 |
|---|---|
| Batch 없음 | 대기 상태로 진입, UI에 "배치 시작 필요" 표시 |
| Batch 있고 Plan 없음 | 해당 종의 Preset으로 자동 생성 |
| Plan 손상 | PlanInvalid Event + Preset 재생성 |
| Preset 자체 실패 | SafeMode 진입 |

---

## 16. 구현 순서

```
Step 1. 기반 설정
        ProductIds.h → PinConfig.h → ProductEventCodes.h

Step 2. HAL 구현
        Esp32Clock → Esp32Watchdog → Esp32NvsStorage → Esp32I2cBus → LgfxConfig

Step 3. Device 구현
        Aht20Driver → Aht20TempDevice / Aht20HumiDevice (Aht20SensorDevice.h + Aht20TempHumiDevices.cpp)
        GpioRelayDevice → PwmFanDevice
        St7789DisplayDevice → Ec11InputDevice
        DeviceRegistry
        (Phase 2+: IMotionDevice·스테퍼 전란, PlanSync·원격 저장소)

Step 4. 도메인 모델
        IncubationSpecies → IncubationPlanRow → IncubationPlanTable
        IncubationBatch → AppSettings → RuntimeState → PlanSyncState

Step 5. 도메인 정책
        IIncubationPresetPolicy → ChickenPresetPolicy → (기타 조류)
        IncubationPolicyFactory → IncubationPlanGenerator
        IncubationDayResolver

Step 6. 저장소
        IIncubationPlanRepository → LocalPlanRepository

Step 7. Observability
        SerialTraceSink → BuzzerAlarmSink

Step 8. Product Module
        IncubationScheduleModule → ClimateControlModule → TurningControlModule

Step 9. UI
        UiModel → UiController → MainUiRenderer

Step 10. 조립
         RegisterServices.cpp → main.cpp

Step 11. Phase 2 확장
         PlanSyncService → RemotePlanRepository
         FwCore AddProvisioning, AddCloud, AddOta 활성화
```

---

## 17. 개발 단계 (Phase)

### Phase 1 — 로컬 독립 운전 (완전 자립)

```ini
# platformio.ini: 별도 build_flags 없음
```

- 모든 기능이 로컬에서 완결
- 클라우드 의존 없이 부화 운전 가능
- USB 시리얼 TraceLogger로 디버깅

### Phase 2 — 클라우드 연동

```ini
build_flags =
    -D Incubator_ENABLE_PROVISIONING
    -D Incubator_ENABLE_CLOUD
```

- BLE 프로비저닝 (WiFi 자격 증명)
- AWS IoT Core 연결
- Blazor WASM 원격 UI
- 로컬/원격 동기화 (PlanSyncService)

### Phase 3 — OTA + 고도화

```ini
build_flags =
    ...
    -D Incubator_ENABLE_OTA
```

- OTA 펌웨어 업데이트
- 카카오톡 알람 (Lambda 경유)
- PID 기반 온도 제어 (rule-based → PID)

---

## 18. 미결 사항 (Open Issues)

| # | 항목 | 상태 | 결정 기준 |
|---|---|---|---|
| OI-001 | 환기팬 제어 — 히터 연동 vs 독립 스케줄 | 미결 | Phase 1 구현 중 결정 |
| OI-002 | 전란 방향 제어 — 단방향 vs 왕복 | 미결 | Phase 2+ `IMotionDevice`(스테퍼) 도입 시 확정 |
| OI-003 | 동기화 충돌 UX — 자동 병합 vs 사용자 선택 | 미결 | Phase 2 진입 전 결정 |
| OI-004 | 알람 채널 — KakaoTalk API 방식 | 미결 | Phase 2 Lambda 설계 시 결정 |
| OI-005 | Custom Preset 데이터 입력 경로 — 로컬 UI vs 원격 전용 | 미결 | UI 설계 시 결정 |

---

## 변경 이력

| 버전 | 날짜 | 내용 |
|---|---|---|
| 1.0 | 2026-05-01 | 초안 작성 (21_Incubator_Starter.md 기반 리팩토링) |
| 1.0.1 | 2026-05-03 | §1.1 로컬 TFT 사양 명시 (GMT020-02-7P, 7핀, BL 핀 없음), §5·부트 다이어그램 디스플레이 파일명 `St7789DisplayDevice`로 정정 |
| 1.0.2 | 2026-05-03 | §5 `include/incubator`·`src` 분리 반영, §5.1·§6·§14·§16·§4.1 정합 (`incubator::config`, `GpioRelayDevice`, AHT20 경로, Phase 2 표기) |
| 1.0.3 | 2026-05-03 | §5 `components`에 
(`22_Incubator_DetailDesign`) 교차 참고 문단 추가 |
| 1.0.4 | 2026-05-03 | §15 부트 시퀀스: Arduino `Wire` 표기 제거, `Esp32I2cBus`/SPI·소스 기준으로 정정 |
| 1.0.5 | 2026-05-03 | §18 OI-002 담당 구현 표기를 Phase 2+ `IMotionDevice`(스테퍼)로 정정 |
| 1.0.6 | 2026-05-03 | §7~§8·§11 도메인 경로를 `include/incubator/domain/...`로 정리, §7.5~7.6은 `22` §5~6 교차 참조로 통일 |
| 1.0.7 | 2026-05-03 | §7.2~7.4·7.7·§8.1~8.4 스니펫을 `lockdownStartDay`·네임스페이스·인터페이스 시그니처 등 저장소 헤더와 동기화 |
| 1.0.8 | 2026-05-03 | §9~§11 `IncubationPlanGenerator`·`IncubationDayResolver`·Repository·JSON에 `lockdownStartDay` 등 저장소와 동기화, §10.3 Phase 1 SSR 전란 명시 |
| 1.0.9 | 2026-05-03 | 상단 문서 `Version` 메타를 2.0으로 통일. 이후 주요 본문 수정 시 상단 `Version`과 본 변경 이렐을 함께 갱신한다 (`00_PLATFORM_BIBLE` §8 항 6). |

---

*이 문서는 FwCore.Common 00_PLATFORM_BIBLE.md의 하위 문서이며, 충돌 시 PLATFORM_BIBLE이 우선한다.*
