#include <Arduino.h>

#include "product/domain/RuntimeState.h"

#include "product/cloud/WifiManager.h"
#include "product/cloud/AwsIotClient.h"

#include "product/time/TimeService.h"

using namespace incubator;

domain::RuntimeState runtime;

cloud::WifiManager wifi;

cloud::AwsIotClient aws;

time::TimeService timeService;

void setup()
{
    Serial.begin(115200);

    wifi.begin(
        "MY_WIFI",
        "12345678");

    aws.begin();

    timeService.begin();

    runtime.currentTempC = 37.5f;

    runtime.currentHumidityPct = 60.0f;
}

void loop()
{
    const uint32_t now = millis();

    timeService.tick(now);

    wifi.tick();

    aws.tick();

    if ((now % 5000) < 50)
    {
        aws.publishTelemetry(runtime);
    }
}

extern "C" void app_main()
{
    initArduino();

    setup();

    while (true)
    {
        loop();
    }
}