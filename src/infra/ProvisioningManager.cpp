#include "infra/ProvisioningManager.h"
#include <Arduino.h>
#include <WiFi.h>
#include <esp_log.h>
#include <esp_mac.h>
#include <cstdio>
#include <cstring>

#if CONFIG_BLUEDROID_ENABLED
#include <WiFiProv.h>
#include <wifi_provisioning/manager.h>
#endif

namespace incubator::infra
{
namespace
{
    const char* TAG = "Provisioning";
}

void ProvisioningManager::init()
{
    buildDeviceIdentity();
    if (m_settings.wifiConfigured) {
        WiFi.mode(WIFI_STA);
        WiFi.begin();
        m_state = ProvisioningState::Idle;
    }
}

void ProvisioningManager::startBootProvisioning(uint32_t nowMs)
{
    if (m_settings.wifiConfigured) return;
    start(nowMs, m_settings.bootProvisionTimeoutMs, false);
}

void ProvisioningManager::requestMenuProvisioning(uint32_t nowMs)
{
    start(nowMs, m_settings.menuProvisionTimeoutMs, true);
}

void ProvisioningManager::start(uint32_t nowMs, uint32_t timeoutMs, bool resetProvisioned)
{
    buildDeviceIdentity();
    m_startedAtMs = nowMs;
    m_timeoutMs = timeoutMs;
    m_active = true;
    m_state = ProvisioningState::Advertising;

#if CONFIG_BLUEDROID_ENABLED
    ESP_LOGI(TAG, "Starting BLE provisioning: name=%s pop=%s timeout=%ums",
             m_deviceName, m_pop, static_cast<unsigned>(timeoutMs));
    WiFi.mode(WIFI_STA);
    WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE,
                            WIFI_PROV_SCHEME_HANDLER_FREE_BTDM,
                            WIFI_PROV_SECURITY_1,
                            m_pop,
                            m_deviceName,
                            nullptr,
                            nullptr,
                            resetProvisioned);
    m_startedStack = true;
#else
    (void)resetProvisioned;
    ESP_LOGE(TAG, "BLE provisioning requested, but CONFIG_BLUEDROID_ENABLED is disabled");
    m_state = ProvisioningState::Failed;
    m_active = false;
#endif
}

void ProvisioningManager::cancel()
{
    if (!m_active) return;
    stopProvisioning();
    m_active = false;
    m_state = ProvisioningState::LocalMode;
    ESP_LOGI(TAG, "Provisioning cancelled; local mode");
}

void ProvisioningManager::tick(uint32_t nowMs)
{
    if (WiFi.status() == WL_CONNECTED) {
        markConnected();
        return;
    }

    if (!m_active) return;
    if (m_timeoutMs > 0U && nowMs - m_startedAtMs >= m_timeoutMs) {
        stopProvisioning();
        m_active = false;
        m_state = ProvisioningState::TimedOut;
        ESP_LOGW(TAG, "Provisioning timed out; local mode");
    }
}

bool ProvisioningManager::isConnected() const
{
    return WiFi.status() == WL_CONNECTED;
}

uint32_t ProvisioningManager::remainingMs(uint32_t nowMs) const
{
    if (!m_active || m_timeoutMs == 0U) return 0U;
    uint32_t elapsed = nowMs - m_startedAtMs;
    if (elapsed >= m_timeoutMs) return 0U;
    return m_timeoutMs - elapsed;
}

const char* ProvisioningManager::statusText() const
{
    switch (m_state) {
        case ProvisioningState::Advertising: return "BLE 연결 대기";
        case ProvisioningState::Saving: return "WiFi 저장 중";
        case ProvisioningState::Succeeded: return "WiFi 연결 완료";
        case ProvisioningState::TimedOut: return "시간 초과";
        case ProvisioningState::LocalMode: return "로컬 모드";
        case ProvisioningState::Failed: return "BLE 설정 실패";
        default: return "대기";
    }
}

void ProvisioningManager::stopProvisioning()
{
#if CONFIG_BLUEDROID_ENABLED
    if (m_startedStack) {
        wifi_prov_mgr_stop_provisioning();
        wifi_prov_mgr_deinit();
        m_startedStack = false;
    }
#endif
}

void ProvisioningManager::buildDeviceIdentity()
{
    if (m_deviceName[0] != '\0') return;

    uint8_t mac[6] = {};
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    std::snprintf(m_deviceName, sizeof(m_deviceName), "INCUBATOR_%02X%02X%02X",
                  mac[3], mac[4], mac[5]);
    std::snprintf(m_pop, sizeof(m_pop), "%02X%02X%02X%02X",
                  mac[2], mac[3], mac[4], mac[5]);
}

void ProvisioningManager::markConnected()
{
    if (!m_settings.wifiConfigured) {
        m_settings.wifiConfigured = true;
        std::strncpy(m_settings.wifiSsid, WiFi.SSID().c_str(), sizeof(m_settings.wifiSsid) - 1U);
        m_nvs.saveBlob(storage::NvsStorage::kKeySettings, &m_settings, sizeof(m_settings));
    }
    if (m_active || m_state != ProvisioningState::Succeeded) {
        stopProvisioning();
        m_active = false;
        m_state = ProvisioningState::Succeeded;
        ESP_LOGI(TAG, "WiFi connected: %s", WiFi.localIP().toString().c_str());
    }
}

} // namespace incubator::infra
