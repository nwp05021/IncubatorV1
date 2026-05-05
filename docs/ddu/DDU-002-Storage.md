# DDU-002 — Storage Layer (NVS + SPIFFS)
> **Document ID**: DDU-002  
> **Version**: 1.0  
> **상위 문서**: INC-IMPL-001 §8  
> **의존 DDU**: DDU-001 (domain 모델 헤더 필요)  
> **Codex 작업 시간 예상**: 10~15분

---

## 작업 목표

이 DDU 완료 후:
- NVS에 구조체 blob 저장/로드가 동작한다
- SPIFFS에 `plan.json`을 ArduinoJson으로 직렬화/역직렬화한다
- 저장소 레이어가 완전히 독립적으로 테스트 가능하다

---

## 1. 생성할 파일 목록

```
include/storage/NvsStorage.h
src/storage/NvsStorage.cpp

include/storage/PlanStorage.h
src/storage/PlanStorage.cpp
```

---

## 2. NvsStorage

### 2.1 헤더

```cpp
// include/storage/NvsStorage.h
#pragma once
#include <nvs_flash.h>
#include <nvs.h>
#include <cstdint>
#include <cstddef>

namespace incubator::storage
{
    class NvsStorage
    {
    public:
        // NVS 네임스페이스 — 변경 금지
        static constexpr const char* kNamespace = "incubator";

        // NVS 키 상수 — 변경 금지 (변경 시 기존 저장 데이터 못 읽음)
        static constexpr const char* kKeySettings   = "settings";
        static constexpr const char* kKeyBatch      = "batch";
        static constexpr const char* kKeyBootCount  = "boot_cnt";
        static constexpr const char* kKeyRtState    = "rt_state";   // 영속 필드만

        bool init();

        // Blob (구조체 직렬화)
        bool saveBlob(const char* key, const void* data, size_t size);
        bool loadBlob(const char* key, void* data, size_t expectedSize);

        // 단순 값
        bool saveU32(const char* key, uint32_t value);
        bool loadU32(const char* key, uint32_t& outValue);

        bool eraseKey(const char* key);
        bool eraseAll();   // 팩토리 리셋 전용

        bool isInitialized() const { return m_open; }

    private:
        nvs_handle_t m_handle = 0;
        bool         m_open   = false;

        bool openRW();
        void close();
    };
}
```

### 2.2 구현

```cpp
// src/storage/NvsStorage.cpp
#include "storage/NvsStorage.h"
#include <esp_log.h>
#include <cstring>

static const char* TAG = "NvsStorage";

namespace incubator::storage
{

bool NvsStorage::init()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition erased, reinitializing");
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_init failed: %s", esp_err_to_name(err));
        return false;
    }
    return openRW();
}

bool NvsStorage::openRW()
{
    if (m_open) return true;
    esp_err_t err = nvs_open(kNamespace, NVS_READWRITE, &m_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(err));
        return false;
    }
    m_open = true;
    return true;
}

void NvsStorage::close()
{
    if (m_open) {
        nvs_close(m_handle);
        m_open = false;
    }
}

bool NvsStorage::saveBlob(const char* key, const void* data, size_t size)
{
    if (!m_open) return false;
    esp_err_t err = nvs_set_blob(m_handle, key, data, size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "saveBlob[%s] failed: %s", key, esp_err_to_name(err));
        return false;
    }
    err = nvs_commit(m_handle);
    return (err == ESP_OK);
}

bool NvsStorage::loadBlob(const char* key, void* data, size_t expectedSize)
{
    if (!m_open) return false;
    size_t size = expectedSize;
    esp_err_t err = nvs_get_blob(m_handle, key, data, &size);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "loadBlob[%s] not found (first boot)", key);
        return false;
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "loadBlob[%s] failed: %s", key, esp_err_to_name(err));
        return false;
    }
    if (size != expectedSize) {
        ESP_LOGW(TAG, "loadBlob[%s] size mismatch: got %u, expected %u",
                 key, (unsigned)size, (unsigned)expectedSize);
        return false;
    }
    return true;
}

bool NvsStorage::saveU32(const char* key, uint32_t value)
{
    if (!m_open) return false;
    esp_err_t err = nvs_set_u32(m_handle, key, value);
    if (err != ESP_OK) return false;
    return nvs_commit(m_handle) == ESP_OK;
}

bool NvsStorage::loadU32(const char* key, uint32_t& outValue)
{
    if (!m_open) return false;
    esp_err_t err = nvs_get_u32(m_handle, key, &outValue);
    return (err == ESP_OK);
}

bool NvsStorage::eraseKey(const char* key)
{
    if (!m_open) return false;
    esp_err_t err = nvs_erase_key(m_handle, key);
    if (err == ESP_ERR_NVS_NOT_FOUND) return true;  // 이미 없으면 성공
    if (err != ESP_OK) return false;
    return nvs_commit(m_handle) == ESP_OK;
}

bool NvsStorage::eraseAll()
{
    if (!m_open) return false;
    esp_err_t err = nvs_erase_all(m_handle);
    if (err != ESP_OK) return false;
    return nvs_commit(m_handle) == ESP_OK;
}

} // namespace incubator::storage
```

---

## 3. PlanStorage

### 3.1 헤더

```cpp
// include/storage/PlanStorage.h
#pragma once
#include "domain/IncubationPlanTable.h"

namespace incubator::storage
{
    class PlanStorage
    {
    public:
        // SPIFFS 마운트 경로 — 변경 금지
        static constexpr const char* kMountPoint = "/spiffs";
        static constexpr const char* kPlanPath   = "/spiffs/plan.json";

        bool init();   // SPIFFS 마운트

        bool save(const domain::IncubationPlanTable& table);
        bool load(domain::IncubationPlanTable& table);
        bool exists() const;
        bool erase();

        bool isInitialized() const { return m_mounted; }

    private:
        bool m_mounted = false;
    };
}
```

### 3.2 구현

```cpp
// src/storage/PlanStorage.cpp
// ArduinoJson v6 사용.
// JSON 필드명은 INC-IMPL-001 §8.3 에 고정. 변경 금지.
#include "storage/PlanStorage.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <esp_log.h>

static const char* TAG = "PlanStorage";

// JSON 필드명 상수 — INC-IMPL-001 §8.3 단일 진실
static constexpr const char* kFTableVersion    = "tableVersion";
static constexpr const char* kFLastUpdatedAt   = "lastUpdatedAt";
static constexpr const char* kFRowCount        = "rowCount";
static constexpr const char* kFRows            = "rows";
static constexpr const char* kFDay             = "day";
static constexpr const char* kFTargetTempC     = "targetTempC";
static constexpr const char* kFTargetHumPct    = "targetHumidityPct";
static constexpr const char* kFTurningEnabled  = "turningEnabled";
static constexpr const char* kFTurningInterval = "turningIntervalMin";
static constexpr const char* kFVentFan         = "ventFanEnabled";
static constexpr const char* kFUserOverridden  = "userOverridden";

namespace incubator::storage
{

bool PlanStorage::init()
{
    if (!SPIFFS.begin(true, kMountPoint)) {
        ESP_LOGE(TAG, "SPIFFS mount failed");
        return false;
    }
    m_mounted = true;
    ESP_LOGI(TAG, "SPIFFS mounted, total=%u used=%u",
             (unsigned)SPIFFS.totalBytes(), (unsigned)SPIFFS.usedBytes());
    return true;
}

bool PlanStorage::save(const domain::IncubationPlanTable& table)
{
    if (!m_mounted) return false;

    // 최대 35 rows, row당 약 120바이트 → 4200 + overhead
    StaticJsonDocument<6144> doc;

    doc[kFTableVersion]  = table.tableVersion;
    doc[kFLastUpdatedAt] = table.lastUpdatedAt;
    doc[kFRowCount]      = table.rowCount;

    JsonArray rows = doc.createNestedArray(kFRows);
    for (uint8_t i = 0; i < table.rowCount; ++i) {
        const auto& r = table.rows[i];
        JsonObject obj = rows.createNestedObject();
        obj[kFDay]             = r.day;
        obj[kFTargetTempC]     = r.targetTempC;
        obj[kFTargetHumPct]    = r.targetHumidityPct;
        obj[kFTurningEnabled]  = r.turningEnabled;
        obj[kFTurningInterval] = r.turningIntervalMin;
        obj[kFVentFan]         = r.ventFanEnabled;
        obj[kFUserOverridden]  = r.userOverridden;
    }

    File f = SPIFFS.open(kPlanPath, "w");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open %s for write", kPlanPath);
        return false;
    }
    size_t written = serializeJson(doc, f);
    f.close();

    ESP_LOGI(TAG, "Plan saved: %u rows, %u bytes", table.rowCount, (unsigned)written);
    return written > 0;
}

bool PlanStorage::load(domain::IncubationPlanTable& table)
{
    if (!m_mounted) return false;
    if (!exists()) return false;

    File f = SPIFFS.open(kPlanPath, "r");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open %s for read", kPlanPath);
        return false;
    }

    StaticJsonDocument<6144> doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err) {
        ESP_LOGE(TAG, "JSON parse error: %s", err.c_str());
        return false;
    }

    table.tableVersion  = doc[kFTableVersion]  | 0;
    table.lastUpdatedAt = doc[kFLastUpdatedAt] | 0UL;
    table.rowCount      = 0;

    JsonArray rows = doc[kFRows].as<JsonArray>();
    if (rows.isNull()) {
        ESP_LOGW(TAG, "No rows array in plan.json");
        return false;
    }

    for (JsonObject obj : rows) {
        if (table.rowCount >= domain::IncubationPlanTable::kMaxDays) break;
        auto& r = table.rows[table.rowCount++];
        r.day                = obj[kFDay]             | 0;
        r.targetTempC        = obj[kFTargetTempC]     | 37.5f;
        r.targetHumidityPct  = obj[kFTargetHumPct]    | 55.0f;
        r.turningEnabled     = obj[kFTurningEnabled]  | true;
        r.turningIntervalMin = obj[kFTurningInterval] | 120;
        r.ventFanEnabled     = obj[kFVentFan]         | true;
        r.userOverridden     = obj[kFUserOverridden]  | false;
    }

    ESP_LOGI(TAG, "Plan loaded: %u rows, version=%u", table.rowCount, table.tableVersion);
    return table.rowCount > 0;
}

bool PlanStorage::exists() const
{
    if (!m_mounted) return false;
    return SPIFFS.exists(kPlanPath);
}

bool PlanStorage::erase()
{
    if (!m_mounted) return false;
    if (!exists()) return true;
    return SPIFFS.remove(kPlanPath);
}

} // namespace incubator::storage
```

---

## 완료 기준 (Acceptance Criteria)

| # | 항목 | 기준 |
|---|---|---|
| AC-1 | NVS 초기화 | `nvs.init()` 반환 true, Serial에 "NVS OK" 확인 |
| AC-2 | NVS 저장/로드 | AppSettings 저장 후 재부팅 → 동일 값 복원 |
| AC-3 | SPIFFS 마운트 | `planStorage.init()` 반환 true |
| AC-4 | Plan JSON 저장 | 21행 저장 후 `/spiffs/plan.json` 존재 확인 |
| AC-5 | Plan JSON 로드 | 로드 후 rowCount==21, 첫 행 targetTempC==37.8 확인 |
| AC-6 | JSON 필드명 | INC-IMPL-001 §8.3 의 필드명과 100% 일치 |
| AC-7 | 파싱 실패 처리 | 손상된 JSON 로드 시 false 반환, 크래시 없음 |
