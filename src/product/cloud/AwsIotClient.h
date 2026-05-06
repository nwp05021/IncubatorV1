#pragma once

#include "../domain/RuntimeState.h"

namespace incubator::cloud
{
    class AwsIotClient
    {
    public:
        void begin();

        void tick();

        void publishTelemetry(
            const incubator::domain::RuntimeState& runtime);

        bool connected() const;

    private:
        bool m_connected = false;

        uint32_t m_lastPublishMs = 0;
    };
}
