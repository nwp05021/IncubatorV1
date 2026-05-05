# DDU-005 — 제어 모듈 (SensorManager / Scheduler / Climate / Turning)
> **Document ID**: DDU-005  
> **Version**: 1.0  
> **상위 문서**: INC-IMPL-001 §10  
> **의존 DDU**: DDU-001, DDU-002, DDU-003, DDU-004  
> **Codex 작업 시간 예상**: 20~25분

---

## 작업 목표

이 DDU 완료 후:
- AHT20 2단계 비동기 측정이 2초 주기로 RuntimeState를 갱신한다
- 매 10초마다 현재 Day 계산 → Plan Row 조회 → 목표값이 RuntimeState에 반영된다
- 히스테리시스 기반 히터/가습기 ON/OFF 제어가 동작한다
- 설정된 간격마다 전란기가 3분간 ON 후 OFF 된다
- Lockdown / SafeMode / batchInactive 상황에서 안전하게 정지한다

---

## 1. 생성할 파일 목록

```
include/modules/SensorManager.h
src/modules/SensorManager.cpp

include/modules/IncubationScheduler.h
src/modules/IncubationScheduler.cpp

include/modules/ClimateModule.h
src/modules/ClimateModule.cpp

include/modules/TurningModule.h
src/modules/TurningModule.cpp
```

---

## 2. SensorManager

```cpp
// include/modules/SensorManager.h
#pragma once
#include "devices/Aht20Driver.h"
#include "domain/RuntimeState.h"
#include <cstdint>

namespace incubator::modules
{
    class SensorManager
    {
    public:
        static constexpr uint32_t kPollIntervalMs  = 2000U;  // 측정 주기
        static constexpr uint32_t kMeasureDelayMs  =   80U;  // AHT20 변환 대기

        SensorManager(devices::Aht20Driver& driver,
                      domain::RuntimeState& state)
            : m_driver(driver), m_state(state) {}

        void tick(uint32_t nowMs);

        bool isHealthy() const { return m_driver.isConnected(); }

    private:
        devices::Aht20Driver& m_driver;
        domain::RuntimeState& m_state;

        enum class Phase { Idle, WaitResult };
        Phase    m_phase     = Phase::Idle;
        uint32_t m_lastPollMs  = 0;
        uint32_t m_triggerMs   = 0;
    };
}
```

**tick() 상태머신**:

```
Phase::Idle:
  if (nowMs - m_lastPollMs >= kPollIntervalMs):
      m_driver.triggerMeasurement()
      m_triggerMs = nowMs
      m_phase = Phase::WaitResult

Phase::WaitResult:
  if (nowMs - m_triggerMs >= kMeasureDelayMs):
      bool ok = m_driver.fetchResult()
      if (ok):
          m_state.currentTempC       = m_driver.getCachedTemp()
          m_state.currentHumidityPct = m_driver.getCachedHumi()
          m_state.tempSensorOk       = true
          m_state.humiSensorOk       = true
      else:
          m_state.tempSensorOk = false
          m_state.humiSensorOk = false
      m_state.uptimeMs = nowMs
      m_lastPollMs = nowMs
      m_phase = Phase::Idle
```

---

## 3. IncubationScheduler

```cpp
// include/modules/IncubationScheduler.h
#pragma once
#include "domain/RuntimeState.h"
#include "domain/IncubationBatch.h"
#include "domain/IncubationPlanTable.h"
#include <cstdint>

namespace incubator::modules
{
    class IncubationScheduler
    {
    public:
        static constexpr uint32_t kTickIntervalMs = 10000U;  // 10초

        IncubationScheduler(domain::RuntimeState&              state,
                            const domain::IncubationBatch&     batch,
                            const domain::IncubationPlanTable& plan)
            : m_state(state), m_batch(batch), m_plan(plan) {}

        void tick(uint32_t nowMs);

    private:
        domain::RuntimeState&              m_state;
        const domain::IncubationBatch&     m_batch;
        const domain::IncubationPlanTable& m_plan;

        uint32_t m_lastMs = 0;

        void applyRow(const domain::IncubationPlanRow& row);
    };
}
```

**tick() 로직**:

```
if (!m_batch.active):
    m_state.batchActive = false
    return

if (nowMs - m_lastMs < kTickIntervalMs): return

// epoch → 현재 day 계산
uint32_t nowEpoch = (uint32_t)time(nullptr)
uint16_t day = DayResolver::resolve(m_batch.startEpoch, nowEpoch, m_batch.totalDays)

m_state.currentDay    = day
m_state.totalDays     = m_batch.totalDays
m_state.batchActive   = true
m_state.lockdownActive = (day >= m_batch.lockdownStartDay)

// Plan Row 조회
const PlanRow* row = m_plan.getRow(day)
if (row == nullptr):
    ESP_LOGE(TAG, "Plan row missing for day %u — entering safe mode", day)
    m_state.safeMode = true
    return

applyRow(*row)
m_lastMs = nowMs

// applyRow():
//   m_state.targetTempC         = row.targetTempC
//   m_state.targetHumidityPct   = row.targetHumidityPct
//   m_state.turningEnabled      = row.turningEnabled && !m_state.lockdownActive
//   m_state.turningIntervalMin  = row.turningIntervalMin
```

---

## 4. ClimateModule

```cpp
// include/modules/ClimateModule.h
#pragma once
#include "domain/RuntimeState.h"
#include "domain/AppSettings.h"
#include "devices/GpioOutput.h"
#include <cstdint>

namespace incubator::modules
{
    class ClimateModule
    {
    public:
        static constexpr uint32_t kTickIntervalMs = 500U;

        ClimateModule(domain::RuntimeState&      state,
                      const domain::AppSettings& settings,
                      devices::GpioOutput&       heater,
                      devices::GpioOutput&       humidifier,
                      devices::GpioOutput&       buzzer)
            : m_state(state), m_settings(settings),
              m_heater(heater), m_humidifier(humidifier), m_buzzer(buzzer) {}

        void tick(uint32_t nowMs);

    private:
        domain::RuntimeState&      m_state;
        const domain::AppSettings& m_settings;
        devices::GpioOutput&       m_heater;
        devices::GpioOutput&       m_humidifier;
        devices::GpioOutput&       m_buzzer;

        uint32_t m_lastMs       = 0;
        uint32_t m_tempAlarmMs  = 0;
        uint32_t m_humiAlarmMs  = 0;

        void controlTemp(uint32_t delta);
        void controlHumidity(uint32_t delta);
        void checkAlarms(uint32_t delta);
        void allOff();
    };
}
```

**tick() 전체 흐름**:

```
uint32_t delta = nowMs - m_lastMs
if (delta < kTickIntervalMs): return
m_lastMs = nowMs

// 안전 정지 조건
if (m_state.safeMode || !m_state.batchActive):
    allOff()
    return

controlTemp(delta)
controlHumidity(delta)
checkAlarms(delta)

// allOff():
//   m_heater.off(), m_humidifier.off(), m_buzzer.off()
//   m_state.heaterOn = m_state.humidifierOn = false
```

**controlTemp() — 히스테리시스**:

```
if (!m_state.tempSensorOk):
    m_heater.off(); m_state.heaterOn = false; return

float cur    = m_state.currentTempC
float target = m_state.targetTempC
float hyst   = m_settings.tempHysteresis

if (cur < target - hyst):
    m_heater.on()
elif (cur > target + hyst):
    m_heater.off()
// else: 유지 (히스테리시스 밴드 내)

m_state.heaterOn = m_heater.isOn()
```

**controlHumidity() — 히스테리시스** (온도와 동일 패턴):

```
if (!m_state.humiSensorOk):
    m_humidifier.off(); m_state.humidifierOn = false; return

float cur    = m_state.currentHumidityPct
float target = m_state.targetHumidityPct
float hyst   = m_settings.humidityHysteresis

if (cur < target - hyst): m_humidifier.on()
elif (cur > target + hyst): m_humidifier.off()

m_state.humidifierOn = m_humidifier.isOn()
```

**checkAlarms()**:

```
// 온도 알람
float tempErr = m_state.currentTempC - m_state.targetTempC
bool  tempOob = (tempErr >  m_settings.tempAlarmHighOffsetC)
             || (tempErr < -m_settings.tempAlarmLowOffsetC)

if (tempOob && m_state.tempSensorOk):
    m_tempAlarmMs += delta
    if (m_tempAlarmMs >= m_settings.alarmConfirmMs):
        m_state.tempAlarmActive = true
        if (m_settings.alarmEnabled) m_buzzer.on()
else:
    m_tempAlarmMs = 0
    m_state.tempAlarmActive = false

// 습도 알람 (동일 패턴)
float humiErr = m_state.currentHumidityPct - m_state.targetHumidityPct
bool  humiOob = (humiErr >  m_settings.humidAlarmHighOffsetPct)
             || (humiErr < -m_settings.humidAlarmLowOffsetPct)

if (humiOob && m_state.humiSensorOk):
    m_humiAlarmMs += delta
    if (m_humiAlarmMs >= m_settings.alarmConfirmMs):
        m_state.humiAlarmActive = true
        if (m_settings.alarmEnabled) m_buzzer.on()
else:
    m_humiAlarmMs = 0
    m_state.humiAlarmActive = false

// 알람 없으면 부저 끄기
if (!m_state.tempAlarmActive && !m_state.humiAlarmActive):
    m_buzzer.off()
```

---

## 5. TurningModule

```cpp
// include/modules/TurningModule.h
#pragma once
#include "domain/RuntimeState.h"
#include "domain/AppSettings.h"
#include "devices/GpioOutput.h"
#include <cstdint>

namespace incubator::modules
{
    class TurningModule
    {
    public:
        static constexpr uint32_t kTickIntervalMs = 1000U;  // 1초

        TurningModule(domain::RuntimeState&      state,
                      const domain::AppSettings& settings,
                      devices::GpioOutput&       turner)
            : m_state(state), m_settings(settings), m_turner(turner) {}

        void tick(uint32_t nowMs);

    private:
        domain::RuntimeState&      m_state;
        const domain::AppSettings& m_settings;
        devices::GpioOutput&       m_turner;

        uint32_t m_lastMs       = 0;
        uint32_t m_turningOnMs  = 0;
        bool     m_isTurning    = false;
    };
}
```

**tick() 전체 로직**:

```
if (nowMs - m_lastMs < kTickIntervalMs): return
m_lastMs = nowMs

// 정지 조건: 안전 모드, 배치 비활성, lockdown, 전란 비활성화
if (!m_state.batchActive      ||
     m_state.lockdownActive   ||
    !m_state.turningEnabled   ||
     m_state.safeMode):
    m_turner.off()
    m_isTurning        = false
    m_state.turnerOn   = false
    m_state.nextTurningInMin = 0
    return

uint32_t intervalMs = (uint32_t)m_state.turningIntervalMin * 60000UL
uint32_t durationMs = (uint32_t)m_settings.turningDurationMin * 60000UL

if (!m_isTurning):
    uint32_t elapsed = nowMs - m_state.lastTurningMs
    // 다음 전란까지 남은 시간 갱신
    if (intervalMs > elapsed):
        m_state.nextTurningInMin = (intervalMs - elapsed) / 60000U
    else:
        m_state.nextTurningInMin = 0

    if (elapsed >= intervalMs):
        // 전란 시작
        m_turner.on()
        m_isTurning              = true
        m_turningOnMs            = nowMs
        m_state.turnerOn         = true
        m_state.lastTurningMs    = nowMs
        m_state.nextTurningInMin = 0
        ESP_LOGI("Turning", "Turner ON for %u min", m_settings.turningDurationMin)
else:
    // 전란 완료 확인
    if (nowMs - m_turningOnMs >= durationMs):
        m_turner.off()
        m_isTurning      = false
        m_state.turnerOn = false
        ESP_LOGI("Turning", "Turner OFF")
```

---

## 완료 기준 (Acceptance Criteria)

| # | 항목 | 기준 |
|---|---|---|
| AC-1 | 센서 갱신 | 2초마다 state.currentTempC 갱신 확인 (Serial 출력) |
| AC-2 | 센서 fault | AHT20 연결 해제 시 state.tempSensorOk == false |
| AC-3 | 스케줄러 Day | epoch 기반 currentDay 정확히 계산 |
| AC-4 | 목표값 적용 | Day 변경 → state.targetTempC 변경 확인 |
| AC-5 | 히터 ON | currentTemp < targetTemp-hyst → heater.on() |
| AC-6 | 히터 OFF | currentTemp > targetTemp+hyst → heater.off() |
| AC-7 | SafeMode 정지 | state.safeMode=true → 히터/가습기 즉시 OFF |
| AC-8 | 알람 확인 대기 | 60초 미만 이탈 시 알람 미발행 |
| AC-9 | 알람 발행 | 60초 이상 이탈 → tempAlarmActive=true, 부저 ON |
| AC-10 | 전란 시작 | intervalMin 경과 후 turner.on() |
| AC-11 | 전란 완료 | durationMin 경과 후 turner.off() |
| AC-12 | Lockdown 정지 | lockdownActive=true → turner.off(), 이후 전란 없음 |
| AC-13 | Tick 내 금지 | malloc/new/delay/vTaskDelay 없음 |
