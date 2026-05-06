# Incubator Firmware — 개발자 통합 레퍼런스

> Version: 1.0 | 문서 기준일: 2025 | 기반 문서: 00 ~ 18

---

## 목차

1. [시스템 철학 및 핵심 원칙](#1-시스템-철학-및-핵심-원칙)
2. [아키텍처 전체 구조](#2-아키텍처-전체-구조)
3. [상태 모델 정의](#3-상태-모델-정의)
4. [AppController & Command 구조](#4-appcontroller--command-구조)
5. [Module 계층 설계](#5-module-계층-설계)
6. [Storage & Recovery 전략](#6-storage--recovery-전략)
7. [UI/UX 구조](#7-uiux-구조)
8. [Cloud 연동 구조](#8-cloud-연동-구조)
9. [Alarm & Notification](#9-alarm--notification)
10. [폴더 구조 & 네이밍 규칙](#10-폴더-구조--네이밍-규칙)
11. [절대 금지 사항 (하드 룰)](#11-절대-금지-사항-하드-룰)
12. [구현 로드맵 (Phase별)](#12-구현-로드맵-phase별)
13. [DDU 작성 규칙](#13-ddu-작성-규칙)

---

## 1. 시스템 철학 및 핵심 원칙

### 1.1 제품 목표

본 시스템은 다음을 수행하는 **독립형 임베디드 제품**이다.

- 정밀 온도/습도 제어
- Day 기반 자동 부화 스케줄
- 자동 전란 제어
- 전원 차단 후 자동 상태 복구
- AWS IoT 원격 관제 (Cloud 없어도 완전 동작)

**목표 품질**: "ESP32 프로젝트"가 아닌 **완성된 상용 장비** 느낌

---

### 1.2 6대 핵심 철학

| 철학 | 내용 |
|------|------|
| **Single Source of Truth** | `RuntimeState`가 현재 시스템 상태의 유일한 진실 |
| **Centralized State Mutation** | 모든 상태 변경은 반드시 `AppController`를 통과 |
| **Non-Blocking** | `delay()` 금지. 전체 시스템은 Tick 기반 |
| **Offline First** | Cloud 없어도 핵심 기능 100% 동작 |
| **Safety First** | 장애 시 출력 차단이 복구보다 우선 |
| **Long-Term Maintainability** | 6개월 후에도 내가 이해 가능한 구조 |

---

### 1.3 Read / Write 분리 원칙

```
Write: AppController만 수행
Read:  UI, Cloud, Renderer, Telemetry 어디서나 가능
```

---

## 2. 아키텍처 전체 구조

### 2.1 계층 구조

```
┌──────────────────────────────┐
│ UI / Cloud                   │  ← 읽기 전용, Command 생성
├──────────────────────────────┤
│ AppController                │  ← 유일한 상태 변경 통로
├──────────────────────────────┤
│ Module Layer                 │  ← 계산/판단 수행
├──────────────────────────────┤
│ Domain / Policy              │  ← 비즈니스 규칙 정의
├──────────────────────────────┤
│ Storage Layer                │  ← NVS / SPIFFS
├──────────────────────────────┤
│ Device Layer                 │  ← GPIO/하드웨어 접근만
└──────────────────────────────┘
```

### 2.2 계층별 역할과 금지사항

| Layer | 역할 | 금지 |
|-------|------|------|
| UI | 상태 표시 | RuntimeState 직접 수정, GPIO 접근 |
| Cloud | 원격 연동, Command 생성 | GPIO 직접 제어, RuntimeState 직접 변경 |
| AppController | Validation, 상태 변경, 저장 요청 | GPIO 제어, UI Draw, Sensor Read |
| Module | 계산, 상태 갱신, 제어 판단 | 정책 판단, Settings 직접 저장 |
| Device | 하드웨어 접근 | 정책 판단, 상태 변경, Alarm 발생 |

### 2.3 Main Loop 구조

```cpp
while(true)
{
    sensor.tick();      // 2000ms 주기
    scheduler.tick();   // 10000ms 주기
    climate.tick();     // 500ms 주기
    turning.tick();     // 1000ms 주기
    appController.tick(); // Command Queue 처리
    ui.tick();          // 100ms 주기
    cloud.tick();       // 60000ms 주기
}
```

**Tick 주기 요약**

| Module | 주기 |
|--------|------|
| Sensor | 2000ms |
| Climate | 500ms |
| Turning | 1000ms |
| Scheduler | 10000ms |
| UI Render | 100ms |
| Cloud Telemetry | 60000ms |

### 2.4 부팅 순서

```
Power On
  → NVS Init
  → AppSettings Load
  → IncubationBatch Load
  → SPIFFS Mount
  → Plan Load & Validation
  → Device Init
  → RuntimeState 초기화
  → SafeMode 판단
  → Main Loop 시작
```

---

## 3. 상태 모델 정의

### 3.1 상태 종류 개요

| 상태 | 역할 | 저장 위치 | 변경 주체 |
|------|------|-----------|-----------|
| `AppSettings` | 정적 운영 설정 | NVS | AppController |
| `RuntimeState` | 현재 실시간 상태 | RAM (저장 안 함) | Modules |
| `IncubationBatch` | 현재 부화 세션 | NVS | AppController |
| `IncubationPlanTable` | Day별 목표 정책 | SPIFFS JSON | AppController |

---

### 3.2 RuntimeState (핵심 구조체)

```cpp
struct RuntimeState
{
    // Sensor
    float currentTempC;
    float currentHumidityPct;

    // Targets
    float targetTempC;
    float targetHumidityPct;

    // Outputs
    bool heaterOn;
    bool humidifierOn;
    bool turnerOn;

    // Batch
    uint16_t currentDay;
    bool lockdown;

    // System
    bool safeMode;
    bool wifiConnected;
    bool awsConnected;

    // Alarm
    bool highTempAlarm;
    bool lowTempAlarm;
    bool highHumidityAlarm;
    bool lowHumidityAlarm;
    bool sensorFail;
};
```

> **원칙**: `RuntimeState`는 RAM Only. 재부팅 시 Modules가 재계산하여 갱신.

---

### 3.3 AppSettings

```cpp
struct AppSettings
{
    float tempHysteresis;        // 범위: 0.1 ~ 2.0
    float humidityHysteresis;    // 범위: 1 ~ 10
    uint32_t telemetryIntervalMs; // >= 10000
    bool cloudEnabled;
    bool alarmEnabled;
    uint16_t turningDurationSec;
};
```

---

### 3.4 IncubationBatch

```cpp
struct IncubationBatch
{
    bool active;
    char species[32];
    uint32_t startEpoch;
    uint16_t totalDays;
    uint16_t lockdownStartDay;
    char batchId[16];
};
```

---

### 3.5 영구 데이터 Schema Version (필수)

```cpp
struct PersistentHeader
{
    uint32_t magic;
    uint16_t schemaVersion;
    uint16_t size;
};
```

모든 NVS/SPIFFS 저장 구조체에 반드시 포함할 것.

---

## 4. AppController & Command 구조

### 4.1 Command 처리 흐름

```
Input (EC11 / AWS Shadow / BLE / Recovery)
  → Command 생성
  → Command Queue (FIFO, 고정 32개)
  → AppController::tick()
    → validate()       ← 실패 시 즉시 거부, 상태 변경 없음
    → mutate()         ← RuntimeState / Settings / Batch 변경
    → persist()        ← NVS / SPIFFS 저장
    → notify()         ← Event 발행 → UI / Cloud Observer
```

### 4.2 Command 구조체

```cpp
enum class CommandType
{
    StartBatch,
    StopBatch,
    UpdateSettings,
    PatchPlanRow,
    ToggleManualHeater,
    ToggleManualHumidifier,
    ClearSafeMode,
    TriggerManualTurning
};

enum class CommandSource { UI, Cloud, BLE, Recovery, Scheduler };

struct Command
{
    CommandType type;
    uint32_t timestamp;
    CommandSource source;
    union
    {
        StartBatchPayload startBatch;
        PatchPlanPayload patchPlan;
        SettingsPayload settings;
    };
};
```

### 4.3 Command Queue 정책

```cpp
struct CommandQueue
{
    Command buffer[32];  // 고정 크기
    uint8_t head;
    uint8_t tail;
};
// Overflow 시: Drop + Alarm (동적 증가 금지)
```

### 4.4 Validation 예시 (AppController 내부)

```cpp
// SafeMode 시 출력 명령 거부
if (runtime.safeMode && cmd.type == CommandType::ToggleManualHeater)
    return reject(cmd);

// Lockdown 중 전란 거부
if (runtime.lockdown && cmd.type == CommandType::TriggerManualTurning)
    return reject(cmd);

// 온도 범위 검증
if (payload.targetTempC < 30.0f || payload.targetTempC > 42.0f)
    return reject(cmd);
```

### 4.5 저장 정책 (persist)

| 상태 | 저장 시점 |
|------|-----------|
| Settings | 변경 즉시 |
| Batch | Start/Stop 즉시 |
| Plan | 수정 즉시 |
| RuntimeState | **저장 안 함** |

### 4.6 이벤트 (notify)

| Event | 의미 |
|-------|------|
| `BatchStarted` | 부화 시작 |
| `BatchCompleted` | 부화 완료 |
| `AlarmRaised` | 알람 발생 |
| `PlanUpdated` | 정책 수정 |
| `WifiConnected` | WiFi 연결 |
| `SafeModeEntered` | SafeMode 진입 |

> **원칙**: Event는 결과 통지이다. Event가 상태를 직접 변경하면 안 된다.

---

## 5. Module 계층 설계

### 5.1 모듈 목록 및 역할

| Module | 역할 | 입력 | 출력 |
|--------|------|------|------|
| `SensorManager` | AHT20 비동기 측정, RuntimeState 갱신 | - | `RuntimeState.currentTempC`, `.currentHumidityPct` |
| `ClimateModule` | 히스테리시스 기반 환경 제어 | `RuntimeState`, `AppSettings` | GPIO(Heater/Humidifier) 상태 |
| `Scheduler` | Batch → currentDay → PlanRow → Target 갱신 | `IncubationBatch`, `PlanTable` | `RuntimeState.targetTempC`, `.targetHumidityPct` |
| `TurningModule` | 인터벌 기반 자동 전란 | `RuntimeState`, `AppSettings` | GPIO(Turner) 상태 |
| `AlarmDetector` | 임계값 초과 감지 → AlarmEvent | `RuntimeState` | `AlarmEvent` |
| `RecoveryModule` | 오류 감지, SafeMode 진입, Command 발행 | `RuntimeState`, Boot 정보 | `Command` (SafeMode 등) |

### 5.2 ClimateModule 히스테리시스 로직 (개념)

```
heaterOn = currentTemp < (targetTemp - hysteresis/2)
heaterOff = currentTemp > (targetTemp + hysteresis/2)
// 사이 구간: 유지 (출력 변경 없음)
```

### 5.3 Scheduler 데이터 흐름

```
IncubationBatch.startEpoch + 현재 시각
  → currentDay 계산
  → PlanTable[currentDay] 조회
  → RuntimeState.targetTempC / targetHumidityPct 갱신
  → lockdown 여부 갱신
```

---

## 6. Storage & Recovery 전략

### 6.1 저장소 분류

| 저장소 | 대상 | 특성 |
|--------|------|------|
| NVS | AppSettings, IncubationBatch, WiFi Credentials, Recovery Flags | 소용량, 중요 |
| SPIFFS | IncubationPlanTable (`plan.json`) | 대용량, 사람이 읽을 수 있음 |
| RAM | RuntimeState | 휘발성, 저장 안 함 |

### 6.2 SPIFFS Atomic Save (필수)

```
1. /spiffs/plan.tmp 생성
2. JSON 직렬화 후 write
3. 파일 close (flush)
4. plan.json → plan.bak 백업
5. plan.tmp → plan.json 교체
6. 검증 load 수행
```

전원 차단 대응: 반드시 이 순서를 지킬 것.

### 6.3 복구 정책

| 상황 | 대응 |
|------|------|
| Settings 없음/손상 | 기본값 생성 + Warning |
| Batch 없음/손상 | 부화 비활성 상태로 시작 |
| Plan 없음 | Preset 재생성 |
| plan.json 손상 | plan.bak 복구 시도 |
| plan.bak도 손상 | Preset 재생성 |
| 재생성 실패 | **SafeMode 진입** |
| Sensor 실패 | **SafeMode 진입** |
| WDT Reset 3회 (10분 내) | **Boot SafeMode** |

### 6.4 SafeMode

**진입 조건**

- AHT20 센서 응답 없음
- Plan 복구 실패
- 장시간 치명 고온 알람
- 반복 WDT 리셋

**SafeMode 동작**

```
Heater     → OFF (강제)
Humidifier → OFF (강제)
Turner     → OFF (강제)
Fan        → 정책 모드 유지
UI         → SafeMode Overlay 표시
Cloud      → reported.safeMode = true
```

**SafeMode 해제 조건** (모두 충족 필요)

```
ClearSafeMode Command 수신
  → Sensor 정상 확인
  → Plan 정상 확인
  → 출력 재개 허용
```

### 6.5 Watchdog

- Timeout: 5초
- 부팅 시 reset reason 확인
- WDT Reset 발생 시 Recovery Flag 기록

### 6.6 Factory Reset 대상

```
NVS Settings / NVS Batch / WiFi Credentials
SPIFFS Plan / Recovery Flags / UI Session
→ 기본 Settings 생성, Batch inactive, Plan empty, 재부팅 권장
```

---

## 7. UI/UX 구조

### 7.1 하드웨어

| 항목 | 내용 |
|------|------|
| Display | ST7789, 320×240, Landscape |
| Input | EC11 Rotary Encoder + Push Button |
| Grid | 40px base (8열 × 6행) |

### 7.2 레이어 구조

```
EC11
  → UiInputController   (입력 해석)
  → UiNavigator         (화면 이동)
  → UiStateMachine      (현재 UI 상태 관리)
  → UiViewModelBuilder  (RuntimeState → UiModel 변환)
  → Renderer            (UiModel → Draw만 수행)
  → Display Driver      (ST7789 출력)
```

> **핵심 원칙**: Renderer는 UiModel만 읽고 그리기만 한다. AppController 직접 호출 금지.

### 7.3 입력 규칙

| 동작 | 의미 |
|------|------|
| Rotate | 이동 |
| Click | 선택 |
| Long Click | 뒤로 |
| Long Hold | 위험 동작 승인 |

### 7.4 화면 구조

```
┌──────────────────────────────┐
│ StatusBar   (시간/Day/WiFi)  │
├──────────────────────────────┤
│ TitleBar                     │
├──────────────────────────────┤
│ Main Content                 │
├──────────────────────────────┤
│ Bottom Action Bar            │
└──────────────────────────────┘
```

### 7.5 5개 핵심 페이지

| Page | 역할 |
|------|------|
| P0 Home | 현재 온도/습도 중심 (제품의 얼굴) |
| P1 Progress | 부화 진행 상태 (Day/ProgressBar/전란) |
| P2 Manual | 수동 제어 (Long Hold 보호) |
| P3 Plan Edit | Day별 목표값 수정 |
| P4 System | 엔지니어링 정보 (WiFi/AWS/Firmware) |

**최대 Navigation Depth: 2단계**

### 7.6 UI 상태 머신

```
Home → Menu → Edit → Confirm
Home → Overlay (Alarm / SafeMode / Dialog / Toast)
```

**Overlay 우선순위**: SafeMode > Alarm > Dialog > Toast

### 7.7 Rendering 원칙

- **Dirty Render**: 상태 변경 시에만 redraw
- Full Refresh 최소화
- Static Buffer 사용 (반복 new/delete 금지)

### 7.8 P0 Home 필수 표시 항목

- 현재 온도 (크게)
- 현재 습도 (크게)
- 목표값
- 현재 Day / 진행률
- 장치 상태 (Heater/Humidifier ON/OFF)

**P0 금지 항목**: Heap, Debug Log, Raw Sensor 값, Boot Count

### 7.9 색상 정책

| 상태 | 색상 |
|------|------|
| 정상 | Neutral |
| Warning | Yellow |
| Alarm | Red |
| SafeMode | Deep Red |

### 7.10 UI Component DDU 목록

| DDU | Component |
|-----|-----------|
| UI-001 | StatusBar |
| UI-002 | TemperatureCard |
| UI-003 | HumidityCard |
| UI-004 | ProgressBar |
| UI-005 | StatusPill |
| UI-006 | AlarmOverlay |
| UI-007 | SafeModeOverlay |
| UI-008 | Toast |
| UI-009 | Dialog |
| UI-010 | FocusRenderer |

---

## 8. Cloud 연동 구조

### 8.1 전체 데이터 흐름

```
RuntimeState
  → TelemetryBuilder → JSON
  → AwsIotClient → AWS IoT Core

AWS desired
  → MQTT Subscribe
  → CmdParser → Command
  → AppController
```

### 8.2 Device Shadow 전략

```
desired  → Command → AppController  (제어 수신)
reported ← RuntimeState             (상태 보고)
```

### 8.3 MQTT Topic

| 방향 | Topic |
|------|-------|
| Publish (Telemetry) | `incubator/{deviceId}/telemetry` |
| Subscribe (Command) | `incubator/{deviceId}/cmd` |
| Shadow Update | `$aws/things/{deviceId}/shadow/update` |
| Shadow Delta | `$aws/things/{deviceId}/shadow/update/delta` |

### 8.4 Telemetry JSON 예시

```json
{
  "deviceId": "INC-001",
  "day": 7,
  "sensor": { "tempC": 37.5, "humidityPct": 65.0 },
  "target": { "tempC": 37.8, "humidityPct": 60.0 },
  "actuator": { "heater": true, "humidifier": false, "turner": false },
  "safeMode": false
}
```

### 8.5 Telemetry 발행 주기

| 조건 | 주기 |
|------|------|
| 정상 | 60초 |
| Alarm 발생 | 즉시 |
| SafeMode 진입 | 즉시 |
| Plan 변경 | 즉시 |

### 8.6 Offline Queue

```
WiFi 단절 시: TelemetryRingBuffer 사용
- 고정 크기 필수
- FIFO
- Overflow: 오래된 데이터 Drop
```

### 8.7 재연결 정책

| 항목 | 값 |
|------|-----|
| WiFi Retry | 30초 |
| MQTT Retry | 15초 |
| TLS Failure | Backoff |

### 8.8 Cloud DDU 목록

| DDU | 역할 |
|-----|------|
| CLOUD-001 | WifiManager |
| CLOUD-002 | MQTT TLS Connect |
| CLOUD-003 | TelemetryBuilder |
| CLOUD-004 | CmdParser |
| CLOUD-005 | ShadowSync |
| CLOUD-006 | Offline Queue |
| CLOUD-007 | Cloud Recovery |

---

## 9. Alarm & Notification

### 9.1 Alarm 종류

| Alarm | 조건 |
|-------|------|
| `HighTemp` | 고온 초과 |
| `LowTemp` | 저온 미달 |
| `HighHumidity` | 고습 초과 |
| `LowHumidity` | 저습 미달 |
| `SensorFail` | AHT20 응답 없음 |
| `PlanCorrupt` | Plan 손상 |

### 9.2 Alarm 상태 흐름

```
RuntimeState
  → AlarmDetector
  → AlarmEvent
  → UI Overlay 표시
  → Cloud Publish (즉시)
```

### 9.3 Notification 계층

| Level | UX |
|-------|-----|
| Info | Toast |
| Warning | Yellow Banner |
| Alarm | Red Overlay |
| Critical | SafeMode |

### 9.4 Alarm UX 요구사항

- 확인 시간(debounce) 이후 활성화 (즉시 반응 방지)
- 사용자 즉시 인지 가능
- 사용자 행동을 유도하는 메시지 포함

### 9.5 Recovery DDU 목록

| DDU | 역할 |
|-----|------|
| REC-001 | Reset Reason Detector |
| REC-002 | Boot Recovery |
| REC-003 | Plan Validation |
| REC-004 | Settings Validation |
| REC-005 | SafeMode Manager |
| REC-006 | Alarm Recovery |
| REC-007 | WDT Recovery |

---

## 10. 폴더 구조 & 네이밍 규칙

### 10.1 프로젝트 구조

```
product/
├── app/          ← AppBootstrap, MainLoop, DI/Wiring
├── controller/   ← AppController, CommandQueue, Validators
├── domain/       ← RuntimeState, AppSettings, Batch, PlanTable, Policies
├── modules/      ← SensorManager, ClimateModule, TurningModule, Alarm, Scheduler
├── services/     ← TimeService, TelemetryService, NotificationService
├── ui/           ← Renderer, UiModel, Navigation, Overlay, Theme
├── cloud/        ← WifiManager, MQTT, Shadow, Telemetry
├── recovery/     ← SafeModeManager, Restore, WDT, Validation
└── devices/      ← Aht20Device, RelayDevice, St7789Device
```

### 10.2 네이밍 규칙

| 종류 | 규칙 | 예시 |
|------|------|------|
| Class | PascalCase | `ClimateModule`, `AppController` |
| Method | camelCase | `processTick()`, `validateCommand()` |
| Member | `m_` prefix | `m_heaterOn`, `m_currentDay` |
| Interface | `I` prefix (선택) | `ISensor`, `IRenderer` |
| Enum | PascalCase | `CommandType::StartBatch` |
| Namespace | `incubator::layer` | `incubator::ui`, `incubator::cloud` |

### 10.3 빌드 환경

| 항목 | 내용 |
|------|------|
| MCU | ESP32-S3 |
| Display | ST7789 320×240 |
| Sensor | AHT20 |
| Framework | ESP-IDF + Arduino |
| Language | C++17 |
| Build | PlatformIO |

---

## 11. 절대 금지 사항 (하드 룰)

### 11.1 코드 금지

```
❌ delay()
❌ vTaskDelay()
❌ blocking while loop
❌ busy wait
❌ 반복 new / delete
❌ 큰 STL 사용
❌ 런타임 메모리 증가
```

### 11.2 구조적 금지

```
❌ UI → RuntimeState 직접 변경
❌ UI → GPIO 직접 제어
❌ Cloud → GPIO 직접 제어
❌ Cloud → RuntimeState 직접 변경
❌ Device Layer → 정책 판단
❌ Device Layer → 상태 변경
❌ Module → Settings 직접 저장
❌ UI → NVS 직접 저장
❌ Cloud → SPIFFS 직접 저장
```

### 11.3 SafeMode 중 허용 명령

```
Heater ON      → 거부
Humidifier ON  → 거부
Turner ON      → 거부
ClearSafeMode  → Validation 후 허용 (Sensor/Plan 정상 확인 필수)
```

---

## 12. 구현 로드맵 (Phase별)

### Phase-01: Foundation (현재 완료 기준)

- 00~18 아키텍처 문서 완료 ✅

---

### Phase-02: Core Runtime

| 구현 항목 | DDU 예시 |
|-----------|----------|
| `RuntimeState` 구조체 정의 | CORE-001 |
| `AppController` + `CommandQueue` | CORE-002 |
| `Scheduler` (Day 계산) | CORE-003 |
| `ClimateModule` (Hysteresis) | CORE-004 |
| `AlarmDetector` | CORE-005 |

**완료 기준**: Sensor 없이 Mock 값으로 Climate 제어 루프 동작 확인

---

### Phase-03: Storage & Recovery

| 구현 항목 | DDU 예시 |
|-----------|----------|
| NVS Settings R/W | REC-004 |
| NVS Batch R/W | REC-002 |
| SPIFFS Plan JSON (Atomic Save) | REC-003 |
| SafeMode Manager | REC-005 |
| Boot Recovery 순서 구현 | REC-002 |
| WDT 설정 및 Reset Reason 감지 | REC-001, REC-007 |

**완료 기준**: 전원 차단 후 재부팅 시 이전 Batch/Plan 복원 확인

---

### Phase-04: UI Foundation

| 구현 항목 | DDU 예시 |
|-----------|----------|
| Display Driver (ST7789) | - |
| UiModel 구조체 | - |
| UiViewModelBuilder | - |
| StatusBar | UI-001 |
| P0 Home (Temperature/Humidity Card) | UI-002, UI-003 |
| EC11 입력 처리 | - |
| UiNavigator + UiStateMachine | - |
| AlarmOverlay / SafeModeOverlay | UI-006, UI-007 |
| FocusRenderer | UI-010 |

**완료 기준**: P0 화면에서 현재 온도/습도가 정상 표시되고 EC11로 화면 이동 가능

---

### Phase-05: Cloud

| 구현 항목 | DDU |
|-----------|-----|
| WifiManager | CLOUD-001 |
| MQTT TLS Connect | CLOUD-002 |
| TelemetryBuilder | CLOUD-003 |
| CmdParser (JSON → Command) | CLOUD-004 |
| ShadowSync (reported) | CLOUD-005 |
| Offline Queue | CLOUD-006 |

**완료 기준**: WiFi 연결 후 Telemetry 발행, AWS desired 수신 → Command 처리 확인

---

### Phase-06: Product Polish

| 구현 항목 |
|-----------|
| P1 Progress / P3 Plan Edit 화면 |
| 부드러운 Fade/Slide 애니메이션 |
| Trend 표시 (Tiny Graph) |
| Diagnostic 정보 (P4 System) |
| 메모리/성능 최적화 |
| Factory Reset 구현 |

---

## 13. DDU 작성 규칙

### 13.1 DDU 기본 원칙

- 작업 시간: **10분 ~ 40분** 단위
- 독립적, 검증 가능해야 함
- 분명한 책임 범위, 명확한 입력/출력/완료 조건

### 13.2 표준 DDU 템플릿

```markdown
# DDU-XXX — 제목

> Version: 1.0
> Status: Draft
> Dependency: (선행 DDU)
> Estimated Time: XX분

## 목적
이 DDU 완료 후:
- 무엇이 가능한가
- 무엇이 동작하는가

## 생성 파일
include/...
src/...

## 핵심 구조
(상태 흐름 / 입력 / 출력)

## 금지 사항
❌ RuntimeState 직접 수정
❌ delay()
❌ AppController 직접 호출 (Renderer에서)

## Acceptance Criteria
AC-1: ...
AC-2: ...
AC-3: ...
```

### 13.3 Codex/AI 협업 시 필수 제공 정보

| 항목 | 필요 여부 |
|------|-----------|
| 정확한 파일 경로 | ✅ 필수 |
| 입력 구조체 | ✅ 필수 |
| 출력 구조체 | ✅ 필수 |
| 금지 사항 명시 | ✅ 필수 |
| Acceptance Criteria | ✅ 필수 |

**금지**: 전체 제품 생성 요청, 구조 설계 위임, 대규모 자동 생성

---

## 최종 체크리스트

모든 구현 전 다음 질문을 통과할 것:

```
1. 이 상태 변경은 AppController를 통과했는가?
2. 이 상태는 누가 소유하는가?
3. WiFi가 끊겨도 제품은 정상 동작하는가?
4. 전원이 이 순간 꺼져도 다음 부팅에서 안전하게 복구되는가?
5. 이 구조가 장기 유지보수 가능한가?
6. 이 UI 요소가 정말 사용자에게 필요한가?
```

하나라도 "아니오"라면 **구조를 다시 설계한다.**
