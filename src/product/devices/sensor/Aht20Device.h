#pragma once

namespace incubator::devices
{
    class Aht20Device
    {
    public:
        bool begin();

        bool read(
            float& tempC,
            float& humidityPct);
    };
}
