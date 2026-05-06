#pragma once

#include <stdint.h>

namespace incubator::time
{
    class TimeService
    {
    public:
        void begin();

        void tick(
            uint32_t nowMs);

        uint32_t epoch() const;

    private:
        uint32_t m_epoch = 0;

        uint32_t m_lastTickMs = 0;
    };
}
