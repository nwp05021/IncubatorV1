#include "globals.h"
#ifdef INCUBATOR_ENABLE_CLOUD
#include "cloud/AwsIotCredentials.h"
#endif
#include "config/PinConfig.h"
#include "config/AppConfig.h"
#include <nvs_flash.h>
#include <esp_task_wdt.h>
#include <esp_sntp.h>
#include <Arduino.h>
#include <esp_log.h>
#include <cstring>

static const char* TAG = "main";

#ifdef INCUBATOR_ENABLE_CLOUD
namespace
{
    uint32_t s_lastTelemetryMs = 0;
    uint32_t s_lastHealthMs = 0;
}
#endif

namespace g
{
    using namespace incubator;
    using namespace incubator::config;

    devices::I2cBus         i2c;
    devices::Aht20Driver    aht20{i2c};
    devices::GpioOutput     heater{static_cast<gpio_num_t>(Pin::SSR_HEATER)};
    devices::GpioOutput     humidifier{static_cast<gpio_num_t>(Pin::SSR_HUMIDIFIER)};
    devices::GpioOutput     turner{static_cast<gpio_num_t>(Pin::RELAY_TURNER)};
    devices::GpioOutput     buzzer{static_cast<gpio_num_t>(Pin::BUZZER)};
    devices::GpioOutput     fan{static_cast<gpio_num_t>(Pin::FAN_PWM)};
    //devices::PwmFan         fan{Pin::FAN_PWM, static_cast<ledc_channel_t>(Pin::FAN_PWM_CH)};
    devices::St7789Display  display;
    devices::Ec11Encoder    encoder{Pin::ENC_A, Pin::ENC_B, Pin::ENC_BTN};

    domain::AppSettings         settings = domain::AppSettings::defaults();
    domain::RuntimeState        state = domain::RuntimeState::zero();
    domain::IncubationBatch     batch;
    domain::IncubationPlanTable plan;

    storage::NvsStorage    nvs;
    storage::PlanStorage   planStorage;

    modules::SensorManager       sensorMgr{aht20, state};
    modules::IncubationScheduler scheduler{state, batch, plan};
    modules::ClimateModule       climate{state, settings, heater, humidifier, buzzer};
    modules::TurningModule       turning{state, settings, plan, turner};

    app::AppController   appCtrl{state, settings, batch, plan, nvs, planStorage,
                                 heater, humidifier, turner, fan};
    infra::ProvisioningManager provisioning{settings, nvs};

    ui::UiModel        uiModel;
    ui::UiController   uiCtrl{uiModel, state, plan, appCtrl, provisioning, encoder};
    ui::MainUiRenderer renderer{uiModel, display};

#ifdef INCUBATOR_ENABLE_CLOUD
    cloud::WifiManager   wifiMgr;
    cloud::AwsIotClient  awsClient;
#endif
}

void setup()
{
    Serial.begin(115200);
    ESP_LOGI(TAG, "=== Incubator FW %s boot ===", INCUBATOR_FW_VERSION);

    if (!g::nvs.init()) {
        ESP_LOGE(TAG, "NVS init failed — halting");
        while (true) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    g::appCtrl.restoreFromStorage();
    g::provisioning.init();

    if (!g::i2c.init(incubator::config::Pin::I2C_SDA,
                     incubator::config::Pin::I2C_SCL, 400000U)) {
        ESP_LOGE(TAG, "I2C init failed");
    }

    if (!g::aht20.init()) {
        ESP_LOGW(TAG, "AHT20 init failed — sensor fault mode");
    }

    ESP_LOGI(TAG, "Init outputs");
    g::heater.init();
    g::humidifier.init();
    g::turner.init();
    g::buzzer.init();
    g::fan.init();
    ESP_LOGI(TAG, "Outputs initialized");

    ESP_LOGI(TAG, "Init display");
    if (!g::display.init()) {
        ESP_LOGE(TAG, "Display init failed");
    }
    ESP_LOGI(TAG, "Display initialized");

    ESP_LOGI(TAG, "Init encoder");
    g::encoder.init();
    ESP_LOGI(TAG, "Encoder initialized");

    if (!g::planStorage.init()) {
        ESP_LOGE(TAG, "SPIFFS init failed");
    }

    g::appCtrl.validateAndRepairPlan();

#ifdef INCUBATOR_ENABLE_CLOUD
    g::wifiMgr.init(WIFI_SSID, WIFI_PASSWORD);

    g::awsClient.init(
        AWS_IOT_ENDPOINT,
        INCUBATOR_DEVICE_ID,
        incubator::cloud::kAwsRootCaPem,
        incubator::cloud::kAwsDeviceCertPem,
        incubator::cloud::kAwsPrivateKeyPem
    );

    g::awsClient.setCmdCallback(
        [](const char* topic, const char* payload) {
            incubator::cloud::CmdParser::parse(payload, g::appCtrl);
        }
    );
#endif

#ifdef INCUBATOR_ENABLE_CLOUD
    // SNTP가 이미 실행 중인지 확인하여 중복 초기화(Assert 에러) 방지
    if (esp_sntp_enabled() == 0) { 
        ESP_LOGI("main", "Initializing SNTP manually...");
        // 정수 0을 esp_sntp_operatingmode_t 타입으로 명시적 형변환
        esp_sntp_setoperatingmode(static_cast<esp_sntp_operatingmode_t>(SNTP_OPMODE_POLL));
        esp_sntp_setservername(0, "pool.ntp.org");
        esp_sntp_init();
    } else {
        ESP_LOGI("main", "SNTP already running, skipping manual init.");
    }
#endif

    esp_err_t wdtErr = esp_task_wdt_init(static_cast<int>(kWatchdogTimeoutMs / 1000U), true);
    if (wdtErr != ESP_OK && wdtErr != ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "WDT init failed: %s", esp_err_to_name(wdtErr));
    }
    esp_task_wdt_add(nullptr);

    {
        auto& gfx = g::display.gfx();
        gfx.startWrite();
        gfx.fillScreen(0x0000);
        gfx.setTextColor(0xFFFF, 0x0000);
        gfx.setTextSize(2);
        gfx.setCursor(40, 90);
        gfx.print("INCUBATOR");
        gfx.setTextSize(1);
        gfx.setCursor(80, 120);
        gfx.print(INCUBATOR_FW_VERSION);
        gfx.setCursor(50, 140);
        gfx.print("Initializing...");
        gfx.endWrite();
        delay(800);
    }

    g::provisioning.startBootProvisioning(millis());

    ESP_LOGI(TAG, "Setup complete. Boot#%u", g::state.bootCount);
}

void loop()
{
    uint32_t now = millis();
    g::sensorMgr.tick(now);
    g::scheduler.tick(now);
    g::climate.tick(now);
    g::turning.tick(now);
    g::provisioning.tick(now);
    g::encoder.tick(now);
    g::uiCtrl.tick(now);
    g::renderer.render(now);

#ifdef INCUBATOR_ENABLE_CLOUD
    g::wifiMgr.tick(now);
    g::awsClient.tick(now);
    g::state.cloudConnected = g::awsClient.isConnected();
    std::strncpy(g::state.ipAddress, g::wifiMgr.ipAddress(), sizeof(g::state.ipAddress) - 1U);

    if (g::awsClient.isConnected() && now - s_lastTelemetryMs >= kTelemetryMs) {
        char json[1536];
        if (incubator::cloud::TelemetryBuilder::build(
                g::state, g::batch, INCUBATOR_DEVICE_ID, json, sizeof(json)) > 0U) {
            g::awsClient.publishTelemetry(json);
        }
        s_lastTelemetryMs = now;
    }

    if (g::awsClient.isConnected() && now - s_lastHealthMs >= kHealthMs) {
        char json[512];
        if (incubator::cloud::TelemetryBuilder::buildHealth(
                g::state, INCUBATOR_DEVICE_ID, json, sizeof(json)) > 0U) {
            g::awsClient.publishHealth(json, true);
        }
        s_lastHealthMs = now;
    }
#endif

    esp_task_wdt_reset();
}

extern "C" void app_main()
{
    initArduino();
    setup();
    while (true) {
        loop();

        if (g::heater.isOn()) {
            g::fan.on();
        } else {
            g::fan.on();
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
