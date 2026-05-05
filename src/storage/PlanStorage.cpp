#include "storage/PlanStorage.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <esp_log.h>
#include <cstring>

static const char* TAG = "PlanStorage";
static constexpr const char* kFTableVersion  = "tableVersion";
static constexpr const char* kFLastUpdatedAt = "lastUpdatedAt";
static constexpr const char* kFRowCount      = "rowCount";
static constexpr const char* kFRows          = "rows";
static constexpr const char* kFDay           = "day";
static constexpr const char* kFTemp          = "targetTempC";
static constexpr const char* kFHumi          = "targetHumidityPct";
static constexpr const char* kFTurning       = "turningEnabled";
static constexpr const char* kFInterval      = "turningIntervalMin";
static constexpr const char* kFVent           = "ventFanEnabled";
static constexpr const char* kFOverride      = "userOverridden";

namespace incubator::storage
{

bool PlanStorage::init()
{
    if (!SPIFFS.begin(true, kMountPoint)) {
        ESP_LOGE(TAG, "SPIFFS mount failed");
        m_mounted = false;
        return false;
    }
    m_mounted = true;
    ESP_LOGI(TAG, "SPIFFS mounted, total=%u used=%u",
             static_cast<unsigned>(SPIFFS.totalBytes()),
             static_cast<unsigned>(SPIFFS.usedBytes()));
    return true;
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

bool PlanStorage::save(const domain::IncubationPlanTable& table)
{
    if (!m_mounted) return false;
    DynamicJsonDocument doc(8192);
    doc[kFTableVersion] = table.tableVersion;
    doc[kFLastUpdatedAt] = table.lastUpdatedAt;
    doc[kFRowCount] = table.rowCount;
    auto rows = doc.createNestedArray(kFRows);
    for (uint16_t i = 0; i < table.rowCount; ++i) {
        auto row = rows.createNestedObject();
        row[kFDay] = table.rows[i].day;
        row[kFTemp] = table.rows[i].targetTempC;
        row[kFHumi] = table.rows[i].targetHumidityPct;
        row[kFTurning] = table.rows[i].turningEnabled;
        row[kFInterval] = table.rows[i].turningIntervalMin;
        row[kFVent] = table.rows[i].ventFanEnabled;
        row[kFOverride] = table.rows[i].userOverridden;
    }

    File file = SPIFFS.open(kPlanPath, FILE_WRITE);
    if (!file) {
        ESP_LOGE(TAG, "open plan file failed");
        return false;
    }
    if (serializeJson(doc, file) == 0) {
        file.close();
        ESP_LOGE(TAG, "serializeJson failed");
        return false;
    }
    file.close();
    return true;
}

bool PlanStorage::load(domain::IncubationPlanTable& table)
{
    if (!m_mounted) return false;
    if (!exists()) return false;
    File file = SPIFFS.open(kPlanPath, FILE_READ);
    if (!file) {
        ESP_LOGE(TAG, "open plan file failed");
        return false;
    }

    DynamicJsonDocument doc(8192);
    DeserializationError err = deserializeJson(doc, file);
    file.close();
    if (err) {
        ESP_LOGE(TAG, "deserializeJson failed: %s", err.c_str());
        return false;
    }

    if (!doc.containsKey(kFRows)) return false;
    table.clear();
    table.tableVersion = doc[kFTableVersion] | 0;
    table.lastUpdatedAt = doc[kFLastUpdatedAt] | 0U;
    table.rowCount = 0;

    auto rows = doc[kFRows].as<JsonArray>();
    for (JsonObject row : rows) {
        if (table.rowCount >= domain::IncubationPlanTable::kMaxRows) break;
        auto& target = table.rows[table.rowCount];
        target.day = row[kFDay] | 0;
        target.targetTempC = row[kFTemp] | 0.0f;
        target.targetHumidityPct = row[kFHumi] | 0.0f;
        target.turningEnabled = row[kFTurning] | false;
        target.turningIntervalMin = row[kFInterval] | 0;
        target.ventFanEnabled = row[kFVent] | false;
        target.userOverridden = row[kFOverride] | false;
        table.rowCount++;
    }

    return table.isValid();
}

} // namespace incubator::storage
