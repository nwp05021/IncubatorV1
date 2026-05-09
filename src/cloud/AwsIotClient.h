#pragma once
#include <functional>
#include <cstdint>

#ifdef INCUBATOR_ENABLE_CLOUD
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#endif

namespace incubator::cloud
{
    using CmdCallback = std::function<void(const char* topic, const char* payload)>;

    class AwsIotClient
    {
    public:
        static constexpr uint32_t kReconnectIntervalMs = 15000U;
        static constexpr int      kMqttPort = 8883;

        bool init(const char* endpoint,
                  const char* deviceId,
                  const char* rootCaPem,
                  const char* certPem,
                  const char* keyPem);

        void tick(uint32_t nowMs);
        bool publish(const char* topic, const char* json);
        bool isConnected() const { return m_connected; }
        void setCmdCallback(CmdCallback cb) { m_cmdCb = cb; }

    private:
        char     m_deviceId[32] = {};
        bool     m_connected = false;
        uint32_t m_lastReconnectMs = 0;
        CmdCallback m_cmdCb;

#ifdef INCUBATOR_ENABLE_CLOUD
        WiFiClientSecure m_secureClient;
        PubSubClient     m_client{m_secureClient};
        static AwsIotClient* s_instance;
        void reconnect();
        static void mqttCallback(char* topic, uint8_t* payload, unsigned int len);
#endif
    };
}
