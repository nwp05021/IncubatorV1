#pragma once

namespace incubator::cloud
{
    struct CloudState
    {
        bool wifiConnected = false;

        bool mqttConnected = false;

        bool shadowConnected = false;
    };
}
