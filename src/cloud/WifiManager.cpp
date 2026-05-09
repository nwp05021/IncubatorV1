#ifdef INCUBATOR_ENABLE_CLOUD
#include "cloud/WifiManager.h"
#include <WiFi.h>
#include <esp_log.h>

static const char* TAG = "WifiManager";

namespace incubator::cloud
{

bool WifiManager::init(const char* ssid, const char* password)
{
    m_ssid = ssid;
    m_password = password;
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    m_lastRetryMs = 0;
    m_connected = false;
    return true;
}

void WifiManager::tick(uint32_t nowMs)
{
    if (WiFi.status() == WL_CONNECTED) {
        m_connected = true;
        return;
    }

    m_connected = false;
    if (nowMs - m_lastRetryMs < kRetryIntervalMs) {
        return;
    }

    m_lastRetryMs = nowMs;
    WiFi.disconnect();
    WiFi.begin(m_ssid, m_password);
    ESP_LOGI(TAG, "Retrying WiFi");
}

bool WifiManager::isConnected() const
{
    return m_connected;
}

const char* WifiManager::ipAddress() const
{
    if (!m_connected) return "";
    return WiFi.localIP().toString().c_str();
}

} // namespace incubator::cloud
#endif
