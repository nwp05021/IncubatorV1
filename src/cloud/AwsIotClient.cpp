#ifdef INCUBATOR_ENABLE_CLOUD
#include "cloud/AwsIotClient.h"
#include <esp_log.h>
#include <cstring>
#include <cstdio>

namespace incubator::cloud
{
namespace
{
    const char* TAG = "AwsIotClient";

    bool copyChunk(char* dst, size_t dstSize, int offset, const char* src, int len)
    {
        if (!dst || !src || offset < 0 || len < 0) return false;
        if (static_cast<size_t>(offset + len) >= dstSize) return false;
        std::memcpy(dst + offset, src, static_cast<size_t>(len));
        dst[offset + len] = '\0';
        return true;
    }
}

bool AwsIotClient::init(const char* endpoint,
                        const char* deviceId,
                        const char* rootCaPem,
                        const char* certPem,
                        const char* keyPem)
{
    if (!endpoint || !deviceId || !rootCaPem || !certPem || !keyPem) {
        ESP_LOGE(TAG, "Missing AWS IoT configuration");
        return false;
    }

    std::strncpy(m_deviceId, deviceId, sizeof(m_deviceId) - 1);
    std::snprintf(m_uri, sizeof(m_uri), "mqtts://%s:%d", endpoint, kMqttPort);
    std::snprintf(m_cmdTopic, sizeof(m_cmdTopic), "incubator/%s/cmd", m_deviceId);
    std::snprintf(m_telemetryTopic, sizeof(m_telemetryTopic), "incubator/%s/telemetry", m_deviceId);
    std::snprintf(m_healthTopic, sizeof(m_healthTopic), "incubator/%s/health", m_deviceId);

    esp_mqtt_client_config_t mqttCfg = {};
    mqttCfg.uri = m_uri;
    mqttCfg.client_id = m_deviceId;
    mqttCfg.lwt_topic = m_healthTopic;
    mqttCfg.lwt_msg = "{\"status\":\"offline\"}";
    mqttCfg.lwt_qos = kMqttQos;
    mqttCfg.lwt_retain = 1;
    mqttCfg.keepalive = 60;
    mqttCfg.user_context = this;
    mqttCfg.buffer_size = 2048;
    mqttCfg.cert_pem = rootCaPem;
    mqttCfg.client_cert_pem = certPem;
    mqttCfg.client_key_pem = keyPem;
    mqttCfg.reconnect_timeout_ms = 10000;
    mqttCfg.network_timeout_ms = 10000;

    m_client = esp_mqtt_client_init(&mqttCfg);
    if (!m_client) {
        ESP_LOGE(TAG, "esp_mqtt_client_init failed");
        return false;
    }

    esp_err_t err = esp_mqtt_client_register_event(
        m_client, MQTT_EVENT_ANY, &AwsIotClient::mqttEventHandler, this);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_mqtt_client_register_event failed: %s", esp_err_to_name(err));
        return false;
    }

    err = esp_mqtt_client_start(m_client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_mqtt_client_start failed: %s", esp_err_to_name(err));
        return false;
    }

    ESP_LOGI(TAG, "AWS IoT MQTT started: %s", m_uri);
    return true;
}

void AwsIotClient::mqttEventHandler(void* handlerArgs,
                                    esp_event_base_t,
                                    int32_t eventId,
                                    void* eventData)
{
    auto* self = static_cast<AwsIotClient*>(handlerArgs);
    if (!self || !eventData) return;

    auto* event = static_cast<esp_mqtt_event_handle_t>(eventData);
    event->event_id = static_cast<esp_mqtt_event_id_t>(eventId);
    self->handleMqttEvent(event);
}

void AwsIotClient::handleMqttEvent(esp_mqtt_event_handle_t event)
{
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            m_connected = true;
            ESP_LOGI(TAG, "MQTT connected");
            esp_mqtt_client_subscribe(m_client, m_cmdTopic, kMqttQos);
            publishOnlineStatus();
            break;

        case MQTT_EVENT_DISCONNECTED:
            m_connected = false;
            ESP_LOGW(TAG, "MQTT disconnected");
            break;

        case MQTT_EVENT_DATA:
            handleData(event);
            break;

        case MQTT_EVENT_ERROR:
            m_connected = false;
            if (event->error_handle) {
                ESP_LOGW(TAG, "MQTT error: type=%d esp=0x%x tls=0x%x sock=%d",
                         static_cast<int>(event->error_handle->error_type),
                         static_cast<unsigned>(event->error_handle->esp_tls_last_esp_err),
                         static_cast<unsigned>(event->error_handle->esp_tls_stack_err),
                         event->error_handle->esp_transport_sock_errno);
            }
            break;

        default:
            break;
    }
}

void AwsIotClient::handleData(esp_mqtt_event_handle_t event)
{
    if (event->current_data_offset == 0) {
        m_rxTotalLen = event->total_data_len;
        if (event->topic_len <= 0 ||
            static_cast<size_t>(event->topic_len) >= sizeof(m_rxTopic) ||
            static_cast<size_t>(m_rxTotalLen) >= sizeof(m_rxPayload)) {
            ESP_LOGW(TAG, "Dropping oversized MQTT message");
            m_rxTotalLen = 0;
            return;
        }
        copyChunk(m_rxTopic, sizeof(m_rxTopic), 0, event->topic, event->topic_len);
    }

    if (m_rxTotalLen <= 0) return;
    if (!copyChunk(m_rxPayload, sizeof(m_rxPayload),
                   event->current_data_offset, event->data, event->data_len)) {
        ESP_LOGW(TAG, "Dropping fragmented MQTT message");
        m_rxTotalLen = 0;
        return;
    }

    if (event->current_data_offset + event->data_len >= m_rxTotalLen) {
        if (m_cmdCb) {
            m_cmdCb(m_rxTopic, m_rxPayload);
        }
        m_rxTotalLen = 0;
    }
}

void AwsIotClient::publishOnlineStatus()
{
    publishHealth("{\"status\":\"online\"}", true);
}

void AwsIotClient::tick(uint32_t)
{
}

bool AwsIotClient::publish(const char* topic, const char* json)
{
    if (!m_client || !m_connected || !topic || !json) return false;
    return esp_mqtt_client_publish(m_client, topic, json, 0, kMqttQos, 0) >= 0;
}

bool AwsIotClient::publishTelemetry(const char* json)
{
    return publish(m_telemetryTopic, json);
}

bool AwsIotClient::publishHealth(const char* json, bool retain)
{
    if (!m_client || !m_connected || !json) return false;
    return esp_mqtt_client_publish(m_client, m_healthTopic, json, 0, kMqttQos, retain ? 1 : 0) >= 0;
}

} // namespace incubator::cloud
#endif
