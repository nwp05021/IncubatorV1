#pragma once

#include "CloudState.h"
#include "AwsIotClient.h"
#include "TelemetryBuilder.h"

namespace incubator::cloud
{
    class ShadowSyncManager
    {
    public:
        ShadowSyncManager(
            CloudState& state,
            AwsIotClient& aws,
            TelemetryBuilder& builder);

    public:
        void tick(uint32_t nowMs);

    private:
        uint32_t m_lastPublishMs = 0;

        static constexpr uint32_t PublishIntervalMs =
            60000;

    private:
        CloudState& m_state;

        AwsIotClient& m_aws;

        TelemetryBuilder& m_builder;
    };
}