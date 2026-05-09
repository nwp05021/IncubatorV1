#include "globals.h"
#include "config/PinConfig.h"
#include "config/AppConfig.h"
#include <nvs_flash.h>
#include <esp_task_wdt.h>
#include <esp_sntp.h>
#include <Arduino.h>
#include <esp_log.h>

static const char* TAG = "main";

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
    devices::PwmFan         fan{Pin::FAN_PWM, static_cast<ledc_channel_t>(Pin::FAN_PWM_CH)};
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

    extern const uint8_t aws_root_ca_pem_start[];
    extern const uint8_t cert_pem_crt_start[];
    extern const uint8_t private_pem_key_start[];

    g::awsClient.init(
        AWS_IOT_ENDPOINT,
        INCUBATOR_DEVICE_ID,
        reinterpret_cast<const char*>(aws_root_ca_pem_start),
        reinterpret_cast<const char*>(cert_pem_crt_start),
        reinterpret_cast<const char*>(private_pem_key_start)
    );

    g::awsClient.setCmdCallback(
        [](const char* topic, const char* payload) {
            incubator::cloud::CmdParser::parse(payload, g::appCtrl);
        }
    );
#endif

#ifdef INCUBATOR_ENABLE_CLOUD
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
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
#endif

    esp_task_wdt_reset();
}

extern "C" void app_main()
{
    initArduino();
    setup();
    while (true) {
        loop();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
