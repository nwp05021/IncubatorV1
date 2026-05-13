#pragma once
#include <functional>
#include <cstdint>

#ifdef INCUBATOR_ENABLE_CLOUD
#include <mqtt_client.h>
#endif

namespace incubator::cloud
{
    using CmdCallback = std::function<void(const char* topic, const char* payload)>;

    class AwsIotClient
    {
    public:
        static constexpr int      kMqttPort = 8883;
        static constexpr int      kMqttQos = 1;

        bool init(const char* endpoint,
                  const char* deviceId,
                  const char* rootCaPem,
                  const char* certPem,
                  const char* keyPem);

        void tick(uint32_t nowMs);
        bool publish(const char* topic, const char* json);
        bool publishTelemetry(const char* json);
        bool publishHealth(const char* json, bool retain = false);
        bool isConnected() const { return m_connected; }
        void setCmdCallback(CmdCallback cb) { m_cmdCb = cb; }
        const char* telemetryTopic() const { return m_telemetryTopic; }
        const char* healthTopic() const { return m_healthTopic; }
        const char* commandTopic() const { return m_cmdTopic; }

    private:
        char m_deviceId[32] = {};
        char m_uri[160] = {};
        char m_cmdTopic[128] = {};
        char m_telemetryTopic[128] = {};
        char m_healthTopic[128] = {};
        char m_rxTopic[128] = {};
        char m_rxPayload[2048] = {};
        int  m_rxTotalLen = 0;
        bool m_connected = false;
        CmdCallback m_cmdCb;

#ifdef INCUBATOR_ENABLE_CLOUD
        esp_mqtt_client_handle_t m_client = nullptr;
        static void mqttEventHandler(void* handlerArgs,
                                     esp_event_base_t base,
                                     int32_t eventId,
                                     void* eventData);
        void handleMqttEvent(esp_mqtt_event_handle_t event);
        void handleData(esp_mqtt_event_handle_t event);
        void publishOnlineStatus();
#endif
    };
}
