#include <Arduino.h>

#include "product/devices/display/St7789DisplayDevice.h"

#include "product/ui/model/HomeUiModel.h"

#include "product/ui/layout/HomeLayout.h"

#include "product/ui/widgets/StatusBarWidget.h"
#include "product/ui/widgets/TemperatureCardWidget.h"
#include "product/ui/widgets/ProgressWidget.h"

using namespace incubator;

devices::St7789DisplayDevice displayDevice;

ui::HomeLayout layout;

ui::StatusBarWidget statusWidget(
    displayDevice);

ui::TemperatureCardWidget tempWidget(
    displayDevice);

ui::ProgressWidget progressWidget(
    displayDevice);

ui::HomeUiModel model;

uint32_t lastUpdateMs = 0;

void setup()
{
    displayDevice.begin();

    displayDevice.clear(0x0000);

    model.tempC = 37.5f;
    model.currentDay = 7;
    model.totalDays = 21;
    model.wifiConnected = true;

    displayDevice.beginFrame();

    statusWidget.render(layout, model);
    tempWidget.render(layout, model);
    progressWidget.render(layout, model);

    displayDevice.endFrame();
}

void loop()
{
    const uint32_t now = millis();

    if ((now - lastUpdateMs) >= 1000)
    {
        lastUpdateMs = now;

        model.tempC += 0.1f;

        if (model.tempC > 38.3f)
        {
            model.tempC = 37.4f;

            model.heaterOn =
                !model.heaterOn;
        }

        displayDevice.beginFrame();

        tempWidget.render(layout, model);

        displayDevice.endFrame();
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