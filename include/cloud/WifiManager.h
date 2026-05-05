#pragma once
#include <cstdint>

#ifdef INCUBATOR_ENABLE_CLOUD
#include <WiFi.h>
#endif

namespace incubator::cloud
{
    class WifiManager
    {
    public:
        static constexpr uint32_t kRetryIntervalMs = 30000U;

        bool init(const char* ssid, const char* password);
        void tick(uint32_t nowMs);
        bool isConnected() const;
        const char* ipAddress() const;

    private:
        uint32_t m_lastRetryMs = 0;
        bool     m_connected = false;
        const char* m_ssid = nullptr;
        const char* m_password = nullptr;
    };
}
