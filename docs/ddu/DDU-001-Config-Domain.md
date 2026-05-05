# DDU-001 — 프로젝트 기반 설정 & 도메인 모델
> **Document ID**: DDU-001  
> **Version**: 1.0  
> **상위 문서**: INC-IMPL-001 §4 §5 §6  
> **의존 DDU**: 없음 (최초 실행)  
> **Codex 작업 시간 예상**: 5~10분

---

## 작업 목표

이 DDU 완료 후:
- 프로젝트 전체 디렉토리 구조가 생성된다
- 모든 도메인 모델 헤더 파일이 완성된다
- `platformio.ini`와 `partitions.csv`가 생성된다
- **컴파일 가능** 상태 (구현 파일 없어도 헤더만으로 빌드 오류 없음)

---

## 1. 생성할 파일 목록

```
platformio.ini                          ← 빌드 설정
partitions.csv                          ← 파티션 테이블
sdkconfig.defaults                      ← ESP-IDF 기본 설정

include/config/PinConfig.h              ← 핀 번호 단일 정의
include/config/AppConfig.h              ← 빌드 상수 / #ifdef 플래그
include/config/ProductIds.h             ← Source ID 상수

include/domain/IncubationSpecies.h      ← enum Species
include/domain/IncubationPlanRow.h      ← Day Plan 행 구조체
include/domain/IncubationPlanTable.h    ← Day Plan 테이블 (헤더+인라인)
include/domain/IncubationBatch.h        ← 배치 정보 구조체
include/domain/AppSettings.h           ← 운영 설정 구조체
include/domain/RuntimeState.h          ← 현재 상태 스냅샷 구조체

include/globals.h                       ← 전역 객체 extern 선언 (본체는 DDU-008)
```

---

## 2. platformio.ini

```ini
[env:esp32s3]
platform  = espressif32
board     = esp32-s3-devkitc-1
framework = arduino, espidf

board_build.filesystem = spiffs
board_build.partitions = partitions.csv

build_unflags = -std=gnu++11
build_flags =
    -std=gnu++17
    -DCORE_DEBUG_LEVEL=3
    -DBOARD_HAS_PSRAM
    ; Phase 2 활성화 시 주석 해제:
    ; -DINCUBATOR_ENABLE_CLOUD
    ; -DINCUBATOR_ENABLE_PROVISIONING

lib_deps =
    lovyan03/LovyanGFX @ ^1.1.12
    bblanchon/ArduinoJson @ ^6.21.5
    knolleary/PubSubClient @ ^2.8.0

monitor_speed    = 115200
monitor_filters  = esp32_exception_decoder
upload_speed     = 921600
```

---

## 3. partitions.csv

```csv
# Name,   Type, SubType, Offset,   Size,     Flags
nvs,      data, nvs,     0x9000,   0x5000,
otadata,  data, ota,     0xe000,   0x2000,
app0,     app,  ota_0,   0x10000,  0x300000,
app1,     app,  ota_1,   0x310000, 0x300000,
spiffs,   data, spiffs,  0x610000, 0x9F0000,
```

---

## 4. sdkconfig.defaults

```
CONFIG_FREERTOS_HZ=1000
CONFIG_ESP_TASK_WDT_TIMEOUT_S=5
CONFIG_SPIRAM_SUPPORT=y
CONFIG_ESP32S3_SPIRAM_SUPPORT=y
CONFIG_I2C_MASTER_ISR_HANDLER_IN_IRAM=y
CONFIG_ARDUINO_LOOP_STACK_SIZE=8192
```

---

## 5. include/config/PinConfig.h

```cpp
#pragma once
#include <cstdint>

namespace incubator::config
{
    struct Pin
    {
        // ── I2C (AHT20) ──────────────────────────────
        static constexpr int I2C_SDA        = 21;
        static constexpr int I2C_SCL        = 22;

        // ── SPI (ST7789 TFT) ─────────────────────────
        static constexpr int TFT_MOSI       = 23;
        static constexpr int TFT_SCLK       = 18;
        static constexpr int TFT_CS         = 5;
        static constexpr int TFT_DC         = 2;
        static constexpr int TFT_RST        = 4;
        // ⚠️ GMT020-02-7P: BL 핀 없음 → LovyanGFX에서 BL=-1

        // ── 디지털 출력 ──────────────────────────────
        static constexpr int SSR_HEATER     = 25;   // HIGH=ON
        static constexpr int SSR_HUMIDIFIER = 26;   // HIGH=ON
        static constexpr int RELAY_TURNER   = 27;   // HIGH=ON
        static constexpr int BUZZER         = -1;   // disabled until hardware pin is verified

        // ── PWM 팬 ────────────────────────────────────
        static constexpr int FAN_PWM        = -1;   // disabled until hardware pin is verified
        static constexpr int FAN_PWM_CH     = 0;    // LEDC 채널 0

        // ── EC11 로터리 엔코더 ────────────────────────
        static constexpr int ENC_A          = 34;   // CLK — ISR
        static constexpr int ENC_B          = 35;   // DT  — ISR
        static constexpr int ENC_BTN        = 36;   // SW  — 내부 풀업, Active LOW
    };
}
```

> ⚠️ **배선 후 실제 핀 번호로 반드시 수정.** 이 파일만 수정하면 전체 반영.

---

## 6. include/config/AppConfig.h

```cpp
#pragma once

// ── 빌드 타임 상수 ────────────────────────────────────────────────
#define INCUBATOR_FW_VERSION        "1.0.0"
#define INCUBATOR_DEVICE_ID_PREFIX  "INC-"

// ── Watchdog ──────────────────────────────────────────────────────
static constexpr uint32_t kWatchdogTimeoutMs = 5000U;

// ── 주기 상수 (모든 모듈이 참조) ──────────────────────────────────
static constexpr uint32_t kSensorPollMs     = 2000U;   // AHT20 측정 주기
static constexpr uint32_t kControlTickMs    =  500U;   // 제어 루프 주기
static constexpr uint32_t kSchedulerTickMs  = 10000U;  // Day 계산 주기
static constexpr uint32_t kUiTickMs         =  100U;   // UI 갱신 주기
static constexpr uint32_t kTelemetryMs      = 60000U;  // Cloud 발행 주기 (Phase 2)

// ── Phase 조건부 컴파일 ──────────────────────────────────────────
// platformio.ini build_flags 로 활성화
// #define INCUBATOR_ENABLE_CLOUD
// #define INCUBATOR_ENABLE_PROVISIONING
```

---

## 7. include/config/ProductIds.h

```cpp
#pragma once
#include <cstdint>

namespace incubator::config
{
    // Sensor Source IDs
    static constexpr uint16_t Sensor_Temp      = 1001U;
    static constexpr uint16_t Sensor_Humidity  = 1002U;

    // Relay / Output IDs
    static constexpr uint16_t Relay_Heater     = 2001U;
    static constexpr uint16_t Relay_Humidifier = 2002U;
    static constexpr uint16_t Relay_Turner     = 2003U;
    static constexpr uint16_t Output_Buzzer    = 2004U;
    static constexpr uint16_t Output_Fan       = 2005U;
}
```

---

## 8. include/domain/IncubationSpecies.h

```cpp
#pragma once
#include <cstdint>

namespace incubator::domain
{
    enum class Species : uint8_t
    {
        Chicken = 0,
        Duck    = 1,
        Quail   = 2,
        Goose   = 3,
        Custom  = 4
    };

    // UI 표시용 문자열 반환
    inline const char* speciesName(Species s)
    {
        switch (s) {
            case Species::Chicken: return "Chicken";
            case Species::Duck:    return "Duck";
            case Species::Quail:   return "Quail";
            case Species::Goose:   return "Goose";
            default:               return "Custom";
        }
    }
}
```

---

## 9. include/domain/IncubationPlanRow.h

```cpp
#pragma once
#include <cstdint>

namespace incubator::domain
{
    struct IncubationPlanRow
    {
        uint16_t day;                 // 1-based (1 = 첫째날)
        float    targetTempC;         // 목표 온도 (°C)
        float    targetHumidityPct;   // 목표 습도 (%)
        bool     turningEnabled;      // 전란 여부
        uint16_t turningIntervalMin;  // 전란 간격 (분)
        bool     ventFanEnabled;      // 환기팬 동작 여부
        bool     userOverridden;      // 사용자 수동 수정 여부

        // ── 팩토리 ────────────────────────────────────────────────
        static IncubationPlanRow make(uint16_t d,
                                      float    temp,
                                      float    humi,
                                      bool     turning,
                                      uint16_t interval,
                                      bool     fan = true)
        {
            return { d, temp, humi, turning, interval, fan, false };
        }
    };
}
```

---

## 10. include/domain/IncubationPlanTable.h

```cpp
#pragma once
#include "IncubationPlanRow.h"
#include <cstdint>
#include <cstring>

namespace incubator::domain
{
    struct IncubationPlanTable
    {
        static constexpr uint8_t kMaxDays = 35;

        uint16_t         tableVersion  = 0;
        uint32_t         lastUpdatedAt = 0;      // Unix epoch
        uint8_t          rowCount      = 0;
        IncubationPlanRow rows[kMaxDays] = {};

        // ── 조회 ──────────────────────────────────────────────────
        const IncubationPlanRow* getRow(uint16_t day) const
        {
            for (uint8_t i = 0; i < rowCount; ++i)
                if (rows[i].day == day) return &rows[i];
            return nullptr;
        }

        IncubationPlanRow* getRowMutable(uint16_t day)
        {
            for (uint8_t i = 0; i < rowCount; ++i)
                if (rows[i].day == day) return &rows[i];
            return nullptr;
        }

        // ── 유효성 ────────────────────────────────────────────────
        bool isValid()  const { return rowCount > 0; }
        bool isEmpty()  const { return rowCount == 0; }

        void clear()
        {
            rowCount     = 0;
            tableVersion = 0;
            memset(rows, 0, sizeof(rows));
        }
    };
}
```

---

## 11. include/domain/IncubationBatch.h

```cpp
#pragma once
#include "IncubationSpecies.h"
#include <cstdint>
#include <cstring>

namespace incubator::domain
{
    struct IncubationBatch
    {
        bool     active           = false;
        Species  species          = Species::Chicken;
        uint32_t startEpoch       = 0;     // 부화 시작 Unix timestamp (초)
        uint16_t totalDays        = 21;    // 총 부화 일수
        uint16_t lockdownStartDay = 19;    // 전란 금지 시작일 (inclusive)
        uint32_t version          = 0;     // 동기화 버전

        char     batchId[16]      = {};    // "INC-001" 형태
        char     note[32]         = {};    // 사용자 메모 (옵션)

        bool isValid() const
        {
            return active
                && totalDays > 0
                && totalDays <= 35
                && lockdownStartDay > 0
                && lockdownStartDay <= totalDays;
        }
    };
}
```

---

## 12. include/domain/AppSettings.h

```cpp
#pragma once
#include <cstdint>

namespace incubator::domain
{
    struct AppSettings
    {
        // ── 온도 제어 ──────────────────────────────────────────────
        float    tempHysteresis          = 0.3f;    // °C 히스테리시스
        float    tempAlarmHighOffsetC    = 1.5f;    // 목표+이 값 초과 → 고온 알람
        float    tempAlarmLowOffsetC     = 1.5f;    // 목표-이 값 미달 → 저온 알람

        // ── 습도 제어 ──────────────────────────────────────────────
        float    humidityHysteresis      = 2.0f;    // % 히스테리시스
        float    humidAlarmHighOffsetPct = 10.0f;   // 고습 알람 오프셋
        float    humidAlarmLowOffsetPct  = 10.0f;   // 저습 알람 오프셋

        // ── 알람 ──────────────────────────────────────────────────
        uint32_t alarmConfirmMs          = 60000U;  // 알람 확인 대기 (ms)
        bool     alarmEnabled            = true;

        // ── 전란 ──────────────────────────────────────────────────
        uint16_t turningDurationMin      = 3U;      // 전란 1회 동작 시간 (분)

        // ── 디스플레이 ─────────────────────────────────────────────
        uint8_t  displayBrightness       = 80U;     // % (미사용 시 무시)

        // ── Cloud (Phase 2) ────────────────────────────────────────
        bool     cloudEnabled            = false;
        uint32_t telemetryIntervalMs     = 60000U;  // 텔레메트리 발행 주기

        // ── 유효성 ────────────────────────────────────────────────
        bool isValid() const
        {
            return tempHysteresis   > 0.0f && tempHysteresis   < 5.0f
                && humidityHysteresis > 0.0f && humidityHysteresis < 10.0f
                && turningDurationMin >= 1   && turningDurationMin <= 30;
        }

        static AppSettings defaults() { return AppSettings{}; }
    };
}
```

---

## 13. include/domain/RuntimeState.h

```cpp
#pragma once
#include <cstdint>
#include <cstring>

namespace incubator::domain
{
    // ★ 읽기 전용 공유 상태.
    //   쓰기는 각 모듈 내부에서만 허용.
    //   UI 는 반드시 복사본(UiModel)을 통해서만 접근한다.
    struct RuntimeState
    {
        // ── 센서 ──────────────────────────────────────────────────
        float    currentTempC        = 0.0f;
        float    currentHumidityPct  = 0.0f;
        bool     tempSensorOk        = false;
        bool     humiSensorOk        = false;

        // ── 출력 상태 ──────────────────────────────────────────────
        bool     heaterOn            = false;
        bool     humidifierOn        = false;
        bool     fanOn               = false;
        bool     turnerOn            = false;

        // ── 현재 적용 중인 목표값 ──────────────────────────────────
        float    targetTempC         = 37.5f;
        float    targetHumidityPct   = 55.0f;
        bool     turningEnabled      = true;
        uint16_t turningIntervalMin  = 120U;

        // ── 부화 진행 ──────────────────────────────────────────────
        uint16_t currentDay          = 0;
        uint16_t totalDays           = 21;
        bool     batchActive         = false;
        bool     lockdownActive      = false;

        // ── 전란 타이머 ────────────────────────────────────────────
        uint32_t lastTurningMs       = 0;
        uint16_t nextTurningInMin    = 0;

        // ── 알람 ──────────────────────────────────────────────────
        bool     tempAlarmActive     = false;
        bool     humiAlarmActive     = false;

        // ── 시스템 ────────────────────────────────────────────────
        bool     safeMode            = false;
        uint32_t uptimeMs            = 0;
        uint32_t bootCount           = 0;

        // ── Cloud (Phase 2) ────────────────────────────────────────
        bool     cloudConnected      = false;
        char     ipAddress[16]       = {};

        static RuntimeState zero()
        {
            RuntimeState s{};
            memset(&s, 0, sizeof(s));
            return s;
        }
    };
}
```

---

## 14. include/globals.h (선언만 — 정의는 DDU-008 main.cpp)

```cpp
#pragma once
// ★ extern 선언만. 정의(실체)는 src/main.cpp 에 있다.
//   다른 파일에서 전역 객체 접근 시 이 헤더만 include한다.

// 전방 선언 대신 각 헤더 include (컴파일러가 타입을 알아야 함)
#include "devices/I2cBus.h"
#include "devices/Aht20Driver.h"
#include "devices/GpioOutput.h"
#include "devices/PwmFan.h"
#include "devices/St7789Display.h"
#include "devices/Ec11Encoder.h"
#include "domain/AppSettings.h"
#include "domain/RuntimeState.h"
#include "domain/IncubationBatch.h"
#include "domain/IncubationPlanTable.h"
#include "storage/NvsStorage.h"
#include "storage/PlanStorage.h"
#include "modules/SensorManager.h"
#include "modules/IncubationScheduler.h"
#include "modules/ClimateModule.h"
#include "modules/TurningModule.h"
#include "app/AppController.h"
#include "ui/UiModel.h"
#include "ui/UiController.h"
#include "ui/MainUiRenderer.h"

namespace g
{
    extern incubator::devices::I2cBus         i2c;
    extern incubator::devices::Aht20Driver    aht20;
    extern incubator::devices::GpioOutput     heater;
    extern incubator::devices::GpioOutput     humidifier;
    extern incubator::devices::GpioOutput     turner;
    extern incubator::devices::GpioOutput     buzzer;
    extern incubator::devices::PwmFan         fan;
    extern incubator::devices::St7789Display  display;
    extern incubator::devices::Ec11Encoder    encoder;

    extern incubator::domain::AppSettings         settings;
    extern incubator::domain::RuntimeState        state;
    extern incubator::domain::IncubationBatch     batch;
    extern incubator::domain::IncubationPlanTable plan;

    extern incubator::storage::NvsStorage    nvs;
    extern incubator::storage::PlanStorage   planStorage;

    extern incubator::modules::SensorManager       sensorMgr;
    extern incubator::modules::IncubationScheduler scheduler;
    extern incubator::modules::ClimateModule       climate;
    extern incubator::modules::TurningModule       turning;

    extern incubator::app::AppController   appCtrl;

    extern incubator::ui::UiModel          uiModel;
    extern incubator::ui::UiController     uiCtrl;
    extern incubator::ui::MainUiRenderer   renderer;
}
```

> ⚠️ globals.h는 의존성이 많아 순환 include 위험이 있다.  
> 각 모듈 헤더에서는 globals.h를 include하지 않는다.  
> globals.h는 main.cpp와 AppController.cpp 에서만 include한다.

---

## 완료 기준 (Acceptance Criteria)

| # | 항목 | 기준 |
|---|---|---|
| AC-1 | 빌드 오류 없음 | `pio run` 시 헤더 파일 컴파일 오류 없음 |
| AC-2 | 네임스페이스 | 모든 파일이 `incubator::` 하위 네임스페이스 사용 |
| AC-3 | 필드명 | INC-IMPL-001 §6 의 필드명과 100% 일치 |
| AC-4 | PinConfig | 모든 핀 번호가 PinConfig.h 에서만 정의됨 |
| AC-5 | 금지 패턴 | 구조체에 비즈니스 로직(계산 함수) 없음 (isValid/defaults 제외) |
