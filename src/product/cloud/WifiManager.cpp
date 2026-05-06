#include "WifiManager.h"

namespace incubator::cloud
{
    void WifiManager::begin(
        const char* ssid,
        const char* password)
    {
        m_connected = true;
    }

    void WifiManager::tick()
    {
    }

    bool WifiManager::connected() const
    {
        return m_connected;
    }
}
