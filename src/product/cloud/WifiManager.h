#pragma once

namespace incubator::cloud
{
    class WifiManager
    {
    public:
        void begin(
            const char* ssid,
            const char* password);

        void tick();

        bool connected() const;

    private:
        bool m_connected = false;
    };
}
