#include "Aht20Device.h"

namespace incubator::devices
{
    bool Aht20Device::begin()
    {
        return true;
    }

    bool Aht20Device::read(
        float& tempC,
        float& humidityPct)
    {
        tempC = 37.6f;

        humidityPct = 61.0f;

        return true;
    }
}
