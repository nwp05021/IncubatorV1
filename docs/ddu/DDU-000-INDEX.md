# DDU Master Index — Incubator Firmware (Standalone)
> **Document ID**: DDU-000  
> **Version**: 1.0 | **Status**: Authoritative  
> **상위 설계 문서**: INC-IMPL-001 (Incubator_Standalone_ImplGuide.md)  
> **목적**: Codex에게 작업을 지시할 때 이 INDEX를 먼저 읽힌다.  
>           각 DDU는 독립 실행 가능한 단위이며 순서대로 진행한다.

---

## Codex 필독 — 작업 시작 전 규칙

```
1. 이 INDEX와 해당 DDU 문서를 먼저 완전히 읽는다.
2. INC-IMPL-001.md 의 관련 섹션을 교차 참조한다.
3. 생성할 파일 목록이 DDU에 명시된 것과 정확히 일치해야 한다.
4. 필드명 / 파일명 / 네임스페이스를 임의로 변경하지 않는다.
5. 각 DDU 말미의 완료 기준(AC)을 모두 만족해야 작업 완료다.
6. 의존 DDU가 표시된 경우, 해당 DDU의 코드가 이미 존재한다고 가정하고 include한다.
```

---

## DDU 목록

| DDU ID | 제목 | INC-IMPL-001 참조 섹션 | 상태 |
|---|---|---|---|
| **DDU-001** | 프로젝트 기반 설정 (Config + Domain Model) | §4, §5, §6 | Phase 1 |
| **DDU-002** | Storage (NVS + SPIFFS) | §8 | Phase 1 |
| **DDU-003** | Device Layer (I2C, AHT20, GPIO, PWM, Encoder, Display) | §9 | Phase 1 |
| **DDU-004** | 부화 정책 (Preset + PlanGenerator + DayResolver) | §7 | Phase 1 |
| **DDU-005** | 제어 모듈 (SensorManager, Scheduler, Climate, Turning) | §10 | Phase 1 |
| **DDU-006** | AppController — 단일 명령 통로 | §11 | Phase 1 |
| **DDU-007** | 로컬 UI (UiModel, UiController, MainUiRenderer 5-Page) | §12 | Phase 1 |
| **DDU-008** | main.cpp + platformio.ini + 부트 시퀀스 | §14, §15 | Phase 1 |
| **DDU-009** | AWS IoT 연동 (Phase 2) | §13 | Phase 2 |

---

## 의존 관계 그래프

```
DDU-001 (Config + Domain)
    │
    ├── DDU-002 (Storage)       ← DDU-001 필요
    ├── DDU-003 (Device)        ← DDU-001 필요
    └── DDU-004 (Policy)        ← DDU-001 필요
              │
              ├── DDU-005 (Modules)   ← DDU-001, 002, 003, 004 필요
              │         │
              │         └── DDU-006 (AppController) ← DDU-001~005 필요
              │                       │
              │                       ├── DDU-007 (UI) ← DDU-001~006 필요
              │                       │
              │                       └── DDU-008 (main.cpp) ← DDU-001~007 필요
              │
              └── DDU-009 (AWS IoT, Phase 2) ← DDU-006 필요
```

---

## 네임스페이스 규칙 (전체 공통)

```cpp
incubator::config     // PinConfig, AppConfig
incubator::domain     // 모든 도메인 모델 구조체
incubator::policy     // Preset, PlanGenerator, DayResolver
incubator::storage    // NvsStorage, PlanStorage
incubator::devices    // 모든 Device 클래스
incubator::modules    // SensorManager, Scheduler, Climate, Turning
incubator::app        // AppController
incubator::ui         // UiModel, UiController, MainUiRenderer
incubator::cloud      // Phase 2 전용
```

---

## 파일 경로 규칙

```
헤더:  include/<namespace_path>/<FileName>.h
구현:  src/<namespace_path>/<FileName>.cpp

예시:
  include/domain/AppSettings.h
  src/modules/ClimateModule.cpp
  include/ui/UiModel.h
```

---

## 공통 금지 패턴

```cpp
// ❌ Tick/loop() 내 금지
malloc(), new, delete, delay(), vTaskDelay()

// ❌ ISR 내 금지
Serial.print(), printf(), EventBus, malloc(), new

// ❌ Device 클래스 내 금지
비즈니스 로직, 정책 판단, 알람 발행

// ❌ UI 렌더러 내 금지
RuntimeState 직접 수정, AppController 직접 호출 (UiController 경유만 허용)

// ❌ 전역 금지
Arduino Wire.begin() 직접 호출 (I2cBus 경유 필수)
FwCore.Common 헤더 include
```
