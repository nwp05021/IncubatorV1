#ifdef INCUBATOR_ENABLE_CLOUD
#include "cloud/AwsIotClient.h"
#include <esp_log.h>
#include <WiFi.h>

namespace incubator::cloud
{

AwsIotClient* AwsIotClient::s_instance = nullptr;

void AwsIotClient::mqttCallback(char* topic, uint8_t* payload, unsigned int len)
{
    if (!s_instance || !s_instance->m_cmdCb) return;
    static char buffer[1024];
    if (len >= sizeof(buffer)) return;
    memcpy(buffer, payload, len);
    buffer[len] = '\0';
    s_instance->m_cmdCb(topic, buffer);
}

bool AwsIotClient::init(const char* endpoint,
                        const char* deviceId,
                        const char* rootCaPem,
                        const char* certPem,
                        const char* keyPem)
{
    std::strncpy(m_deviceId, deviceId, sizeof(m_deviceId) - 1);
    m_secureClient.setCACert(rootCaPem);
    m_secureClient.setCertificate(certPem);
    m_secureClient.setPrivateKey(keyPem);
    m_client.setServer(endpoint, kMqttPort);
    m_client.setCallback(mqttCallback);
    s_instance = this;
    m_lastReconnectMs = 0;
    m_connected = false;
    return true;
}

void AwsIotClient::reconnect()
{
    if (m_client.connected()) return;
    if (!WiFi.isConnected()) return;

    if (m_client.connect(m_deviceId)) {
        m_connected = true;
        char topic[128];
        snprintf(topic, sizeof(topic), "incubator/%s/cmd", m_deviceId);
        m_client.subscribe(topic);
        ESP_LOGI("AwsIotClient", "MQTT connected and subscribed");
    } else {
        ESP_LOGW("AwsIotClient", "MQTT connect failed");
        m_connected = false;
    }
}

void AwsIotClient::tick(uint32_t nowMs)
{
    if (m_client.connected()) {
        m_client.loop();
        m_connected = true;
        return;
    }

    if (nowMs - m_lastReconnectMs < kReconnectIntervalMs) return;
    m_lastReconnectMs = nowMs;
    reconnect();
}

bool AwsIotClient::publish(const char* topic, const char* json)
{
    if (!m_client.connected()) return false;
    return m_client.publish(topic, json);
}

} // namespace incubator::cloud
#endif
