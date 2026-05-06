#include <Arduino.h>

#include "product/devices/display/St7789DisplayDevice.h"

#include "product/ui/model/HomeUiModel.h"
#include "product/ui/screen/GraphicHomeScreen.h"

using namespace incubator;

devices::St7789DisplayDevice displayDevice;

ui::GraphicHomeScreen homeScreen(
    displayDevice);

ui::HomeUiModel homeModel;

uint32_t lastUpdateMs = 0;

void setup()
{
    Serial.begin(115200);

    displayDevice.begin();

    displayDevice.clear(0x0000);

    homeModel.tempC = 37.5f;
    homeModel.humidityPct = 60.0f;
    homeModel.currentDay = 7;
    homeModel.totalDays = 21;
    homeModel.wifiConnected = true;
    homeModel.awsConnected = false;

    homeScreen.invalidate();
    homeScreen.render(homeModel);
}

void loop()
{
    const uint32_t now = millis();

    if ((now - lastUpdateMs) >= 1000)
    {
        lastUpdateMs = now;

        homeModel.tempC += 0.1f;

        if (homeModel.tempC > 38.2f)
        {
            homeModel.tempC = 37.4f;
            homeModel.heaterOn = !homeModel.heaterOn;
        }

        homeModel.humidityPct += 1.0f;

        if (homeModel.humidityPct > 68.0f)
        {
            homeModel.humidityPct = 58.0f;
            homeModel.humidifierOn = !homeModel.humidifierOn;
        }

        homeScreen.render(homeModel);
    }

    vTaskDelay(1);
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