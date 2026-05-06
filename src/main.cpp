#include <Arduino.h>

#include "product/app/AppController.h"
#include "product/app/CommandQueue.h"
#include "product/devices/display/St7789DisplayDevice.h"
#include "product/domain/AppSettings.h"
#include "product/domain/RuntimeState.h"
#include "product/input/Ec11InputDevice.h"
#include "product/input/InputEventQueue.h"
#include "product/input/InputRuntimeTask.h"
#include "product/runtime/RuntimeScheduler.h"
#include "product/ui/model/HomeUiModel.h"
#include "product/ui/screen/GraphicHomeScreen.h"
#include "product/ui/screen/PlaceholderScreen.h"
#include "product/ui/screen_manager/ScreenContext.h"
#include "product/ui/screen_manager/ScreenId.h"
#include "product/ui/screen_manager/ScreenManager.h"

using namespace incubator;

namespace
{
    constexpr int Ec11PinA = 5;
    constexpr int Ec11PinB = 6;
    constexpr int Ec11PinButton = 4;
}

devices::St7789DisplayDevice displayDevice;

ui::GraphicHomeScreen homeScreen(displayDevice);
ui::PlaceholderScreen progressScreen(
    displayDevice,
    ui::ScreenId::Progress,
    "INCUBATION PROGRESS",
    "Day policy and trend view");
ui::PlaceholderScreen settingsScreen(
    displayDevice,
    ui::ScreenId::Settings,
    "SYSTEM SETTINGS",
    "Targets and device profile");
ui::PlaceholderScreen diagnosticsScreen(
    displayDevice,
    ui::ScreenId::Diagnostics,
    "RUNTIME DIAGNOSTICS",
    "Scheduler and health view");
ui::PlaceholderScreen alarmScreen(
    displayDevice,
    ui::ScreenId::Alarm,
    "ALARM CENTER",
    "SafeMode and active faults");

ui::ScreenManager screenManager;
ui::HomeUiModel model;

runtime::RuntimeScheduler runtimeScheduler;
app::CommandQueue commandQueue;
domain::RuntimeState runtimeState;
domain::AppSettings appSettings;
app::AppController appController(
    runtimeState,
    appSettings,
    commandQueue);

input::Ec11InputDevice ec11Device(
    Ec11PinA,
    Ec11PinB,
    Ec11PinButton);
input::InputEventQueue inputQueue;
input::InputRuntimeTask inputRuntimeTask(
    ec11Device,
    inputQueue,
    commandQueue);

uint32_t lastModelUpdateMs = 0;
uint32_t lastScreenChangeMs = 0;
uint8_t demoScreenIndex = 0;
constexpr bool AutoScreenDemoEnabled = false;

static ui::ScreenId screenFromDemoIndex(
    uint8_t index)
{
    switch (index % 5)
    {
        case 0:
            return ui::ScreenId::Home;

        case 1:
            return ui::ScreenId::Progress;

        case 2:
            return ui::ScreenId::Settings;

        case 3:
            return ui::ScreenId::Diagnostics;

        default:
            return ui::ScreenId::Alarm;
    }
}

static void tickInputRuntime(
    uint32_t nowMs)
{
    inputRuntimeTask.tick(nowMs);
}

static void tickAppController(
    uint32_t nowMs)
{
    (void)nowMs;
    appController.tick();
}

void setup()
{
    Serial.begin(115200);

    displayDevice.begin();
    displayDevice.clear(0x0000);

    inputRuntimeTask.begin();
    inputRuntimeTask.setDiagnosticsEnabled(true);

    runtimeScheduler.addTask(
        runtime::RuntimeTaskId::Input,
        runtime::RuntimePriority::Normal,
        "Input",
        5,
        1000,
        tickInputRuntime);

    runtimeScheduler.addTask(
        runtime::RuntimeTaskId::AppController,
        runtime::RuntimePriority::Normal,
        "AppController",
        10,
        1000,
        tickAppController);

    model.tempC = 37.5f;
    model.humidityPct = 55.0f;
    model.currentDay = 7;
    model.totalDays = 21;
    model.wifiConnected = true;
    model.awsConnected = false;
    model.heaterOn = true;
    model.humidifierOn = false;
    model.fanOn = true;
    model.fanPwm = 65;

    screenManager.registerScreen(
        ui::ScreenId::Home,
        homeScreen);

    screenManager.registerScreen(
        ui::ScreenId::Progress,
        progressScreen);

    screenManager.registerScreen(
        ui::ScreenId::Settings,
        settingsScreen);

    screenManager.registerScreen(
        ui::ScreenId::Diagnostics,
        diagnosticsScreen);

    screenManager.registerScreen(
        ui::ScreenId::Alarm,
        alarmScreen);

    screenManager.request(ui::ScreenId::Home);

    Serial.println("[INPUT] EC11 runtime foundation ready");
}

void loop()
{
    const uint32_t now = millis();

    runtimeScheduler.tick(now);

    if ((now - lastModelUpdateMs) >= 1000)
    {
        lastModelUpdateMs = now;

        model.tempC += 0.1f;
        model.humidityPct += 1.0f;

        if (model.tempC > 38.3f)
        {
            model.tempC = 37.4f;
            model.heaterOn = !model.heaterOn;
        }

        if (model.humidityPct > 62.0f)
        {
            model.humidityPct = 54.0f;
            model.humidifierOn = !model.humidifierOn;
        }
    }

    model.focusedItem = runtimeState.focusedItem;

    if (AutoScreenDemoEnabled &&
        (now - lastScreenChangeMs) >= 4000)
    {
        lastScreenChangeMs = now;
        ++demoScreenIndex;

        screenManager.request(
            screenFromDemoIndex(demoScreenIndex));
    }

    ui::ScreenContext context;
    context.home = &model;
    context.nowMs = now;

    screenManager.tick(context);

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
