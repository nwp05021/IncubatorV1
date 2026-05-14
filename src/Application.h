#pragma once
#include <cstdint>
#include <cstring>
#include <Arduino.h>

#include "devices/I2cBus.h"
#include "devices/Aht20Driver.h"
#include "devices/GpioOutput.h"
#include "config/PinConfig.h"
#include "storage/PlanStorage.h"
#include "domain/IncubationPlanTable.h"
#include "domain/SystemState.h"
#include "services/SensorManager.h"
#include "services/SchedulerService.h"
#include "services/ClimateController.h"
#include "services/TurningController.h"
#include "services/EncoderController.h"
#include "services/UiController.h"
#include "ui/DisplayRenderer.h"

#ifdef INCUBATOR_ENABLE_CLOUD
#include "services/WiFiManager.h"
#include "cloud/AwsIotClient.h"
#include "cloud/TelemetryBuilder.h"
#endif

namespace incubator
{
    class Application
    {
    public:
        Application();
        ~Application() = default;

        // 복사 금지
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        /**
         * @brief 하드웨어 및 소프트웨어 서비스들을 순차적으로 초기화합니다.
         */
        void setup();

        /**
         * @brief main loop에서 호출되며 각 컴포넌트들의 주기적 처리를 수행합니다.
         */
        void loop();

    private:
        // 하드웨어 드라이버 컴포넌트
        devices::I2cBus         m_i2c;
        devices::Aht20Driver    m_aht20{m_i2c};
        devices::GpioOutput     m_heater{static_cast<gpio_num_t>(config::Pin::SSR_HEATER)};
        devices::GpioOutput     m_humidifier{static_cast<gpio_num_t>(config::Pin::SSR_HUMIDIFIER)};
        devices::GpioOutput     m_turner{static_cast<gpio_num_t>(config::Pin::RELAY_TURNER)};
        devices::GpioOutput     m_buzzer{static_cast<gpio_num_t>(config::Pin::BUZZER)};
        devices::GpioOutput     m_fan{static_cast<gpio_num_t>(config::Pin::FAN_PWM)};

        // 데이터 스토리지 및 도메인 상태 관리
        storage::PlanStorage          m_storage;
        domain::IncubationPlanTable   m_planTable;
        domain::SystemState           m_state;
        domain::IncubationBatch       m_batch;

        // 코어 비즈니스 서비스 로직
        services::SensorManager       m_sensorMgr{m_aht20, m_state};
        services::SchedulerService    m_scheduler{m_state, m_batch, m_planTable, m_storage};
        services::ClimateController   m_climate{m_heater, m_humidifier, m_fan, m_state, m_scheduler};
        services::TurningController   m_turning{m_turner, m_state, m_scheduler};
        services::EncoderController   m_encoder{static_cast<gpio_num_t>(config::Pin::ENC_A),
                                                static_cast<gpio_num_t>(config::Pin::ENC_B),
                                                static_cast<gpio_num_t>(config::Pin::ENC_BTN)};
        services::UiController        m_uiCtrl{m_state, m_batch, m_planTable, m_scheduler, m_climate, m_turning, m_buzzer};
        ui::DisplayRenderer           m_renderer{m_uiCtrl};

#ifdef INCUBATOR_ENABLE_CLOUD
        services::WiFiManager         m_wifiMgr;
        cloud::AwsIotClient           m_awsClient;
        services::BootProvisioning    m_provisioning{m_wifiMgr, m_awsClient, m_state};

        uint32_t m_lastTelemetryMs = 0;
        uint32_t m_lastHealthMs = 0;

        static constexpr uint32_t kTelemetryMs = 5000U;
        static constexpr uint32_t kHealthMs = 30000U;
#endif
    };
}