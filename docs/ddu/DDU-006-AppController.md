# DDU-006 — AppController (단일 명령 통로)
> **Document ID**: DDU-006  
> **Version**: 1.0  
> **상위 문서**: INC-IMPL-001 §11  
> **의존 DDU**: DDU-001, DDU-002, DDU-004, DDU-005  
> **Codex 작업 시간 예상**: 15~20분

---

## 작업 목표

이 DDU 완료 후:
- UI / AWS IoT 원격 명령 등 모든 외부 명령이 `AppController::applyCommand()` 하나를 통과한다
- 부팅 시 NVS + SPIFFS 에서 상태를 복원한다
- 팩토리 리셋 / SafeMode 해제 / Preset 재생성이 동작한다

---

## 1. 생성할 파일 목록

```
include/app/AppController.h
src/app/AppController.cpp
```

---

## 2. AppController 헤더

```cpp
// include/app/AppController.h
#pragma once
#include "domain/AppSettings.h"
#include "domain/RuntimeState.h"
#include "domain/IncubationBatch.h"
#include "domain/IncubationPlanTable.h"
#include "domain/IncubationPlanRow.h"
#include "domain/IncubationSpecies.h"
#include "storage/NvsStorage.h"
#include "storage/PlanStorage.h"
#include "policy/PlanGenerator.h"
#include <cstdint>
#include <cstddef>

namespace incubator::app
{
    // ── 명령 코드 ─────────────────────────────────────────────────
    // 필드명/값 변경 금지 (Cloud JSON 매핑과 1:1 대응)
    enum class Cmd : uint16_t
    {
        // 배치 관리
        StartBatch      = 100,
        StopBatch       = 101,

        // Plan 편집
        PatchPlanRow    = 200,   // payload: IncubationPlanRow
        ResetPlan       = 201,   // Preset으로 재생성

        // 수동 제어 (Page 2 Manual / 긴급)
        HeaterOn        = 300,
        HeaterOff       = 301,
        HumidOn         = 302,
        HumidOff        = 303,
        TurnerOn        = 304,
        TurnerOff       = 305,
        FanSetDuty      = 306,   // payload: uint8_t duty(0~100)

        // 설정 변경
        UpdateSettings  = 400,   // payload: AppSettings

        // 시스템
        ClearSafeMode   = 500,
        FactoryReset    = 501,
        SyncNow         = 502,   // Phase 2
    };

    // Cloud JSON → Cmd 문자열 매핑 (Phase 2 참조용)
    // "START_BATCH"     → Cmd::StartBatch
    // "STOP_BATCH"      → Cmd::StopBatch
    // "PATCH_PLAN_ROW"  → Cmd::PatchPlanRow
    // "RESET_PLAN"      → Cmd::ResetPlan
    // "UPDATE_SETTINGS" → Cmd::UpdateSettings
    // "CLEAR_SAFE_MODE" → Cmd::ClearSafeMode

    class AppController
    {
    public:
        AppController(domain::RuntimeState&        state,
                      domain::AppSettings&         settings,
                      domain::IncubationBatch&     batch,
                      domain::IncubationPlanTable& plan,
                      storage::NvsStorage&         nvs,
                      storage::PlanStorage&        planStorage)
            : m_state(state), m_settings(settings),
              m_batch(batch), m_plan(plan),
              m_nvs(nvs), m_planStorage(planStorage) {}

        // ★ 모든 명령의 단일 진입점
        bool applyCommand(Cmd cmd,
                          const void* payload   = nullptr,
                          size_t      payloadSz = 0);

        // 부팅 시 저장된 상태 복원 (setup()에서 호출)
        bool restoreFromStorage();

        // Plan 유효성 검사 + 자동 복구 (setup()에서 호출)
        // - Plan 없음 → Preset 자동 생성
        // - Plan 손상 → Preset 재생성
        // - Preset 실패 → safeMode = true
        void validateAndRepairPlan();

    private:
        domain::RuntimeState&        m_state;
        domain::AppSettings&         m_settings;
        domain::IncubationBatch&     m_batch;
        domain::IncubationPlanTable& m_plan;
        storage::NvsStorage&         m_nvs;
        storage::PlanStorage&        m_planStorage;

        // ── 각 명령 핸들러 ─────────────────────────────────────────
        bool cmdStartBatch    (const domain::IncubationBatch&   b);
        bool cmdStopBatch     ();
        bool cmdPatchPlanRow  (const domain::IncubationPlanRow& row);
        bool cmdResetPlan     ();
        bool cmdUpdateSettings(const domain::AppSettings&       s);
        bool cmdFactoryReset  ();
        bool cmdClearSafeMode ();

        // 헬퍼
        void generateBatchId(domain::IncubationBatch& b);
    };
}
```

---

## 3. AppController 구현

### 3.1 applyCommand() — 디스패처

```cpp
// src/app/AppController.cpp
bool AppController::applyCommand(Cmd cmd, const void* payload, size_t sz)
{
    switch (cmd)
    {
    // ── 배치 ──────────────────────────────────────────────────────
    case Cmd::StartBatch:
        if (!payload || sz != sizeof(domain::IncubationBatch)) return false;
        return cmdStartBatch(*static_cast<const domain::IncubationBatch*>(payload));

    case Cmd::StopBatch:
        return cmdStopBatch();

    // ── Plan ──────────────────────────────────────────────────────
    case Cmd::PatchPlanRow:
        if (!payload || sz != sizeof(domain::IncubationPlanRow)) return false;
        return cmdPatchPlanRow(*static_cast<const domain::IncubationPlanRow*>(payload));

    case Cmd::ResetPlan:
        return cmdResetPlan();

    // ── 수동 제어 (RuntimeState 플래그로 ClimateModule/TurningModule에 전달)
    //    ※ 수동 ON 후 다음 ClimateModule tick에서 자동 제어가 우선권 가짐.
    //       완전한 수동 제어가 필요하면 별도 manualOverride 플래그 추가 권장.
    case Cmd::HeaterOn:
        m_state.safeMode = false;
        // ClimateModule이 다음 tick에서 처리 — 직접 GPIO 건드리지 않음
        return true;

    case Cmd::HeaterOff:
        // ClimateModule 의 자동 제어를 일시 무력화하려면
        // safeMode를 잠깐 활성화하는 방법도 있으나 여기서는
        // 간단하게 safeMode 없이 ClimateModule에 맡김
        return true;

    case Cmd::TurnerOn:
        m_state.turnerOn = true;   // TurningModule tick에서 인지
        return true;

    case Cmd::TurnerOff:
        m_state.turnerOn = false;
        return true;

    // ── 설정 ──────────────────────────────────────────────────────
    case Cmd::UpdateSettings:
        if (!payload || sz != sizeof(domain::AppSettings)) return false;
        return cmdUpdateSettings(*static_cast<const domain::AppSettings*>(payload));

    // ── 시스템 ────────────────────────────────────────────────────
    case Cmd::ClearSafeMode:
        return cmdClearSafeMode();

    case Cmd::FactoryReset:
        return cmdFactoryReset();

    default:
        ESP_LOGW("AppCtrl", "Unknown Cmd: %u", (unsigned)cmd);
        return false;
    }
}
```

### 3.2 핵심 핸들러 구현

```cpp
bool AppController::cmdStartBatch(const domain::IncubationBatch& b)
{
    m_batch         = b;
    m_batch.active  = true;
    if (m_batch.batchId[0] == '\0') generateBatchId(m_batch);

    // Plan 재생성 (Preset 기준)
    if (!cmdResetPlan()) return false;

    // NVS 저장
    m_nvs.saveBlob(storage::NvsStorage::kKeyBatch,
                   &m_batch, sizeof(m_batch));
    ESP_LOGI("AppCtrl", "Batch started: %s (%s)",
             m_batch.batchId,
             domain::speciesName(m_batch.species));
    return true;
}

bool AppController::cmdStopBatch()
{
    m_batch.active = false;
    m_nvs.saveBlob(storage::NvsStorage::kKeyBatch,
                   &m_batch, sizeof(m_batch));
    return true;
}

bool AppController::cmdPatchPlanRow(const domain::IncubationPlanRow& row)
{
    auto* r = m_plan.getRowMutable(row.day);
    if (!r) {
        ESP_LOGE("AppCtrl", "PatchPlanRow: day %u not found", row.day);
        return false;
    }
    *r = row;
    r->userOverridden = true;
    m_plan.tableVersion++;

    bool ok = m_planStorage.save(m_plan);
    ESP_LOGI("AppCtrl", "PatchPlanRow: day %u saved (v%u)",
             row.day, m_plan.tableVersion);
    return ok;
}

bool AppController::cmdResetPlan()
{
    m_plan.clear();
    bool ok = policy::PlanGenerator::generate(
                  m_batch.species, m_batch, m_plan);
    if (!ok) {
        ESP_LOGE("AppCtrl", "PlanGenerator failed!");
        m_state.safeMode = true;
        return false;
    }
    m_planStorage.save(m_plan);
    // NVS에 batch도 재저장 (totalDays 변경됐을 수 있음)
    m_nvs.saveBlob(storage::NvsStorage::kKeyBatch,
                   &m_batch, sizeof(m_batch));
    return true;
}

bool AppController::cmdUpdateSettings(const domain::AppSettings& s)
{
    if (!s.isValid()) {
        ESP_LOGW("AppCtrl", "UpdateSettings: invalid values");
        return false;
    }
    m_settings = s;
    return m_nvs.saveBlob(storage::NvsStorage::kKeySettings,
                          &m_settings, sizeof(m_settings));
}

bool AppController::cmdFactoryReset()
{
    m_nvs.eraseAll();
    m_planStorage.erase();
    m_settings = domain::AppSettings::defaults();
    m_batch    = {};
    m_plan.clear();
    m_state    = domain::RuntimeState::zero();
    ESP_LOGW("AppCtrl", "Factory reset complete — reboot recommended");
    return true;
}

bool AppController::cmdClearSafeMode()
{
    m_state.safeMode = false;
    ESP_LOGI("AppCtrl", "SafeMode cleared");
    return true;
}
```

### 3.3 restoreFromStorage()

```cpp
bool AppController::restoreFromStorage()
{
    bool anyFail = false;

    // 1. AppSettings
    domain::AppSettings loaded{};
    if (m_nvs.loadBlob(NvsStorage::kKeySettings, &loaded, sizeof(loaded))
        && loaded.isValid()) {
        m_settings = loaded;
        ESP_LOGI("AppCtrl", "Settings restored");
    } else {
        m_settings = domain::AppSettings::defaults();
        m_nvs.saveBlob(NvsStorage::kKeySettings, &m_settings, sizeof(m_settings));
        ESP_LOGI("AppCtrl", "Settings: defaults applied");
    }

    // 2. BootCount
    uint32_t cnt = 0;
    m_nvs.loadU32(NvsStorage::kKeyBootCount, cnt);
    cnt++;
    m_nvs.saveU32(NvsStorage::kKeyBootCount, cnt);
    m_state.bootCount = cnt;
    ESP_LOGI("AppCtrl", "Boot count: %u", cnt);

    // 3. IncubationBatch
    domain::IncubationBatch b{};
    if (m_nvs.loadBlob(NvsStorage::kKeyBatch, &b, sizeof(b)) && b.isValid()) {
        m_batch = b;
        ESP_LOGI("AppCtrl", "Batch restored: %s", m_batch.batchId);
    }

    return !anyFail;
}
```

### 3.4 validateAndRepairPlan()

```cpp
void AppController::validateAndRepairPlan()
{
    if (!m_batch.active) return;

    // Plan 로드 시도
    bool planLoaded = m_planStorage.load(m_plan);

    if (planLoaded && m_plan.isValid()) {
        ESP_LOGI("AppCtrl", "Plan valid: %u rows", m_plan.rowCount);
        return;
    }

    // Plan 없거나 손상 → Preset으로 재생성
    ESP_LOGW("AppCtrl", "Plan missing/invalid — regenerating from Preset");
    m_plan.clear();
    bool ok = policy::PlanGenerator::generate(
                  m_batch.species, m_batch, m_plan);
    if (ok) {
        m_planStorage.save(m_plan);
        ESP_LOGI("AppCtrl", "Plan regenerated: %u rows", m_plan.rowCount);
    } else {
        ESP_LOGE("AppCtrl", "Plan regeneration failed — SafeMode");
        m_state.safeMode = true;
    }
}
```

### 3.5 generateBatchId() 헬퍼

```cpp
void AppController::generateBatchId(domain::IncubationBatch& b)
{
    uint32_t cnt = m_state.bootCount;
    snprintf(b.batchId, sizeof(b.batchId), "INC-%03u", (unsigned)(cnt % 1000));
}
```

---

## 완료 기준 (Acceptance Criteria)

| # | 항목 | 기준 |
|---|---|---|
| AC-1 | StartBatch | 호출 후 batch.active==true, plan.rowCount>0 |
| AC-2 | PatchPlanRow | row 수정 후 SPIFFS plan.json 변경 확인 |
| AC-3 | PatchPlanRow 재부팅 | 재부팅 후 수정된 row 값 유지 |
| AC-4 | RestoreFromStorage | 재부팅 후 settings, batch 자동 복원 |
| AC-5 | ValidateAndRepair | plan.json 삭제 후 재부팅 → Preset 자동 재생성 |
| AC-6 | SafeMode | Preset 실패 시 state.safeMode == true |
| AC-7 | FactoryReset | NVS / SPIFFS 초기화, 기본값 복원 |
| AC-8 | BootCount | 재부팅마다 +1 증가 |
| AC-9 | 단일 통로 | 모든 명령이 applyCommand() 경유 확인 (직접 state 수정 없음) |
