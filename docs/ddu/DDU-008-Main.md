# DDU-008 — main.cpp & 부트 시퀀스
> **Document ID**: DDU-008  
> **Version**: 1.0  
> **상위 문서**: INC-IMPL-001 §14  
> **의존 DDU**: DDU-001 ~ DDU-007 전부  
> **Codex 작업 시간 예상**: 10~15분

---

## 작업 목표

이 DDU 완료 후:
- 전역 객체가 단 하나의 파일(main.cpp)에만 정의된다
- 부트 시퀀스가 정해진 순서대로 실행된다
- Watchdog이 5초 타임아웃으로 활성화된다
- `loop()`에서 모든 모듈의 `tick()`이 호출된다
- Phase 1 완전 독립 동작 (WiFi 없어도 모든 기능 정상)

---

## 1. 생성할 파일 목록

```
src/main.cpp    ← 전역 객체 정의 + setup() + loop()
```

> **주의**: `include/globals.h`는 DDU-001에서 이미 생성됨.  
> `main.cpp`는 `globals.h`의 extern 선언에 대한 실체(definition)를 제공한다.

---

## 2. src/main.cpp 전체

```cpp
// src/main.cpp
// ★ 전역 객체 정의는 이 파일에만 존재한다.
//   다른 파일에서 전역 객체를 new/static으로 생성 금지.
#include "globals.h"
#include "config/PinConfig.h"
#include "config/AppConfig.h"
#include <nvs_flash.h>
#include <esp_task_wdt.h>
#include <esp_sntp.h>
#include <Arduino.h>
#include <esp_log.h>

static const char* TAG = "main";

// ══════════════════════════════════════════════════════════════════
// 전역 객체 정의 (globals.h의 extern 실체)
// ══════════════════════════════════════════════════════════════════
namespace g
{
    using namespace incubator;
    using namespace incubator::config;

    // ── Device ───────────────────────────────────────────────────
    devices::I2cBus         i2c;
    devices::Aht20Driver    aht20    { i2c };
    devices::GpioOutput     heater   { (gpio_num_t)Pin::SSR_HEATER     };
    devices::GpioOutput     humidifier{ (gpio_num_t)Pin::SSR_HUMIDIFIER };
    devices::GpioOutput     turner   { (gpio_num_t)Pin::RELAY_TURNER   };
    devices::GpioOutput     buzzer   { (gpio_num_t)Pin::BUZZER         };
    devices::PwmFan         fan      { Pin::FAN_PWM,
                                       (ledc_channel_t)Pin::FAN_PWM_CH };
    devices::St7789Display  display;
    devices::Ec11Encoder    encoder  { Pin::ENC_A, Pin::ENC_B, Pin::ENC_BTN };

    // ── Domain ────────────────────────────────────────────────────
    domain::AppSettings         settings  = domain::AppSettings::defaults();
    domain::RuntimeState        state     = domain::RuntimeState::zero();
    domain::IncubationBatch     batch;
    domain::IncubationPlanTable plan;

    // ── Storage ───────────────────────────────────────────────────
    storage::NvsStorage    nvs;
    storage::PlanStorage   planStorage;

    // ── Modules ───────────────────────────────────────────────────
    modules::SensorManager       sensorMgr  { aht20, state };
    modules::IncubationScheduler scheduler  { state, batch, plan };
    modules::ClimateModule       climate    { state, settings,
                                              heater, humidifier, buzzer };
    modules::TurningModule       turning    { state, settings, turner };

    // ── App ───────────────────────────────────────────────────────
    app::AppController   appCtrl { state, settings, batch, plan,
                                   nvs, planStorage };

    // ── UI ────────────────────────────────────────────────────────
    ui::UiModel        uiModel;
    ui::UiController   uiCtrl   { uiModel, state, plan, appCtrl, encoder };
    ui::MainUiRenderer renderer { uiModel, display };
}

// ══════════════════════════════════════════════════════════════════
// setup()
// ══════════════════════════════════════════════════════════════════
void setup()
{
    Serial.begin(115200);
    ESP_LOGI(TAG, "=== Incubator FW %s boot ===", INCUBATOR_FW_VERSION);

    // ── 1. NVS 초기화 ─────────────────────────────────────────────
    if (!g::nvs.init()) {
        ESP_LOGE(TAG, "NVS init failed — halting");
        while (true) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    // ── 2. Storage 복원 (HAL 전 — pin 건드리지 않음) ───────────────
    g::appCtrl.restoreFromStorage();

    // ── 3. I2C (가장 먼저 — AHT20 의존) ──────────────────────────
    if (!g::i2c.init(incubator::config::Pin::I2C_SDA,
                     incubator::config::Pin::I2C_SCL, 400000U)) {
        ESP_LOGE(TAG, "I2C init failed");
        // 계속 진행 — 센서 fault로 처리
    }

    // ── 4. 센서 초기화 ────────────────────────────────────────────
    if (!g::aht20.init()) {
        ESP_LOGW(TAG, "AHT20 init failed — sensor fault mode");
    }

    // ── 5. 출력 초기화 (전원 차단 상태 보장) ──────────────────────
    g::heater.init();       // init() 내부에서 gpio_config + LOW 출력
    g::humidifier.init();
    g::turner.init();
    g::buzzer.init();
    g::fan.init();          // PWM 0%

    // ── 6. 디스플레이 / 입력 ──────────────────────────────────────
    if (!g::display.init()) {
        ESP_LOGE(TAG, "Display init failed");
    }
    g::encoder.init();

    // ── 7. SPIFFS 마운트 ─────────────────────────────────────────
    if (!g::planStorage.init()) {
        ESP_LOGE(TAG, "SPIFFS init failed");
    }

    // ── 8. Plan 유효성 검사 & 자동 복구 ────────────────────────────
    g::appCtrl.validateAndRepairPlan();

    // ── 9. SNTP (Phase 2 없어도 기본 시간 설정 시도) ────────────────
    //   WiFi 없으면 동기화 실패 → epoch = 0 (Day 계산은 상대적)
    //   실제 배포 시 RTC 모듈 추가 권장
#ifdef INCUBATOR_ENABLE_CLOUD
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
#endif

    // ── 10. Watchdog 설정 ──────────────────────────────────────────
    esp_task_wdt_config_t wdt_cfg = {
        .timeout_ms   = kWatchdogTimeoutMs,
        .idle_core_mask = 0,
        .trigger_panic  = true
    };
    esp_task_wdt_reconfigure(&wdt_cfg);
    esp_task_wdt_add(nullptr);

    // ── 11. 부팅 화면 ──────────────────────────────────────────────
    {
        auto& gfx = g::display.gfx();
        gfx.startWrite();
        gfx.fillScreen(0x0000);
        gfx.setTextColor(0xFFFF, 0x0000);
        gfx.setTextSize(2);
        gfx.setCursor(40, 90);
        gfx.print("INCUBATOR");
        gfx.setTextSize(1);
        gfx.setCursor(80, 120);
        gfx.print(INCUBATOR_FW_VERSION);
        gfx.setCursor(50, 140);
        gfx.print("Initializing...");
        gfx.endWrite();
        delay(800);
    }

    ESP_LOGI(TAG, "Setup complete. Boot#%u", g::state.bootCount);
}

// ══════════════════════════════════════════════════════════════════
// loop()
// ★ malloc / new / delay / vTaskDelay 금지
// ★ 모든 타이밍은 millis() 기반 non-blocking
// ══════════════════════════════════════════════════════════════════
void loop()
{
    uint32_t now = millis();

    // 1. 센서 (2000ms 주기 — SensorManager 내부 관리)
    g::sensorMgr.tick(now);

    // 2. 부화 스케줄러 (10000ms 주기)
    g::scheduler.tick(now);

    // 3. 제어 모듈 (500ms 주기)
    g::climate.tick(now);
    g::turning.tick(now);

    // 4. 입력 (매 loop)
    g::encoder.tick(now);

    // 5. UI (200ms sync + 100ms render)
    g::uiCtrl.tick(now);
    g::renderer.render(now);

    // 6. Phase 2: Cloud
#ifdef INCUBATOR_ENABLE_CLOUD
    g::awsClient.tick(now);
#endif

    // 7. Watchdog
    esp_task_wdt_reset();

    // 8. 최소 yield (FreeRTOS idle 기회 부여)
    // loop() 안에서는 blocking delay 금지.
}

extern "C" void app_main()
{
    initArduino();
    setup();
    while (true) {
        loop();
        vTaskDelay(pdMS_TO_TICKS(5));  // FreeRTOS idle 기회 부여
    }
}
```

---

## 3. 부트 시퀀스 다이어그램

```
Power On / Reset
  │
  ├─ Serial.begin(115200)
  ├─ NVS.init()               ← 실패 시 halt
  ├─ AppController.restoreFromStorage()
  │     ├─ NVS → AppSettings
  │     ├─ NVS → IncubationBatch
  │     └─ NVS → BootCount++
  │
  ├─ I2cBus.init(SDA, SCL)
  ├─ Aht20Driver.init()
  │
  ├─ GpioOutput.init() × 4    ← heater/humidifier/turner/buzzer ALL OFF
  ├─ PwmFan.init()             ← duty 0%
  ├─ St7789Display.init()
  ├─ Ec11Encoder.init()
  │
  ├─ PlanStorage.init()        ← SPIFFS 마운트
  ├─ AppController.validateAndRepairPlan()
  │     ├─ Plan 정상   → 그대로 사용
  │     ├─ Plan 없음   → Preset 자동 생성
  │     ├─ Plan 손상   → Preset 재생성
  │     └─ Preset 실패 → state.safeMode = true
  │
  ├─ [Phase 2] SNTP init
  │
  ├─ Watchdog 5초 설정
  ├─ 부팅 화면 표시 (0.8초)
  │
  └─ loop() 진입
        ├─ sensorMgr.tick()     2s
        ├─ scheduler.tick()    10s
        ├─ climate.tick()     500ms
        ├─ turning.tick()       1s
        ├─ encoder.tick()     매회
        ├─ uiCtrl.tick()      200ms
        ├─ renderer.render()  100ms
        ├─ [awsClient.tick() 1min, Phase 2]
        └─ Watchdog reset
```

---

## 4. 시간 기준 설정 (RTC 없는 환경)

WiFi + SNTP 연결 전까지는 `time(nullptr)` 이 epoch 0 근처를 반환한다.  
이 경우 `DayResolver::resolve(0, 0, 21)` → Day 1 로 처리되므로  
**기능은 동작하지만 Day 계산이 부정확하다.**

해결 방법 (우선순위 순):
1. **권장**: 배치 시작 시 앱/원격에서 정확한 `startEpoch`를 설정
2. **대안**: 저가 DS3231 RTC 모듈 추가 (I2C 0x57)
3. **최소**: 사용자가 UI에서 시작 시각을 수동 입력

Phase 1에서는 `startEpoch = millis()/1000` 로 상대 시간 사용 허용.

---

## 완료 기준 (Acceptance Criteria)

| # | 항목 | 기준 |
|---|---|---|
| AC-1 | 빌드 성공 | `pio run` 오류 없음 |
| AC-2 | 업로드 성공 | `pio run -t upload` 정상 |
| AC-3 | 부팅 화면 | TFT에 "INCUBATOR" + 버전 표시 |
| AC-4 | Serial 로그 | "Setup complete. Boot#1" 출력 |
| AC-5 | Watchdog | loop() 평균 < 50ms (5초 WDT 미발동) |
| AC-6 | 복원 | 재부팅 후 이전 batch/settings 자동 복원 |
| AC-7 | Plan 복구 | plan.json 없이 부팅 → Preset 자동 생성 확인 |
| AC-8 | SafeMode | Preset 실패 강제 테스트 시 TFT에 SafeMode 표시 |
| AC-9 | 전체 동작 | 온도/습도 표시 + 히터 제어 + EC11 페이지 이동 |
| AC-10 | 전역 객체 | 전역 객체 정의가 main.cpp 에만 존재 확인 |
