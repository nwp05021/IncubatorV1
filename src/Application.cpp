#include "Application.h"
#include "config/AppConfig.h"
#include <nvs_flash.h>
#include <esp_task_wdt.h>
#include <esp_sntp.h>
#include <esp_log.h>

static const char* TAG = "Application";

namespace incubator
{

Application::Application()
{
    // 생성자에서 기본적인 상태 링크 매핑 처리
    m_state.bootCount = 0U;
    m_state.ipAddress[0] = '\0';
    m_state.cloudConnected = false;
}

void Application::setup()
{
    ESP_LOGI(TAG, "시스템 초기화를 시작합니다...");

    // 1. NVS (Non-Volatile Storage) 초기화
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. 기본 하드웨어 주변장치 제어권 활성화
    m_i2c.init(static_cast<gpio_num_t>(config::Pin::I2C_SDA), 
              static_cast<gpio_num_t>(config::Pin::I2C_SCL));
    m_aht20.init();
    m_heater.init();
    m_humidifier.init();
    m_turner.init();
    m_buzzer.init();
    m_fan.init();

    // 3. 파일 시스템(LittleFS) 구동 및 기존 스토리지 데이터 복원
    if (m_storage.init()) {
        if (m_storage.exists()) {
            m_storage.load(m_planTable);
        }
    } else {
        ESP_LOGE(TAG, "Critical: 스토리지(LittleFS) 제어권 확보 실패");
    }

    // 4. 엔코더, 스케줄러 및 UI 그래픽 엔진 활성화
    m_encoder.init();
    m_scheduler.init();
    m_uiCtrl.init();
    m_renderer.init();

    // 5. 부저 신호음 (부팅 완료 알림)
    m_buzzer.set(true);
    delay(100);
    m_buzzer.set(false);

#ifdef INCUBATOR_ENABLE_CLOUD
    // 6. 클라우드 연동 로직 (LittleFS 마운트 체크 연동 포함)
    m_wifiMgr.init();
    
    // 이전에 리팩토링한 규칙에 맞춤: AWS 엔드포인트와 디바이스ID 입력 및 스토리지 주입
    if (!m_awsClient.init(INCUBATOR_AWS_ENDPOINT, INCUBATOR_DEVICE_ID, m_storage)) {
        ESP_LOGW(TAG, "AWS IoT 인증서 로드 또는 인스턴스 초기화에 실패하여 클라우드가 격리 모드로 작동합니다.");
    }

    m_provisioning.init(INCUBATOR_DEVICE_ID);

    // 내부 네트워크 시간 동기화(SNTP) 시작
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    // 프로비저닝 트리거 작동
    m_provisioning.startBootProvisioning(millis());
#endif

    ESP_LOGI(TAG, "모든 시스템 초기화 프로세스가 완료되었습니다.");
}

void Application::loop()
{
    uint32_t now = millis();

    // 실시간 주기적 장치 갱신(Tick) 처리
    m_sensorMgr.tick(now);
    m_scheduler.tick(now);
    m_climate.tick(now);
    m_turning.tick(now);
    m_encoder.tick(now);
    m_uiCtrl.tick(now);
    m_renderer.render(now);

#ifdef INCUBATOR_ENABLE_CLOUD
    m_wifiMgr.tick(now);
    m_awsClient.tick(now);
    m_provisioning.tick(now);

    // 하위 상태 구조체 동기화
    m_state.cloudConnected = m_awsClient.isConnected();
    std::strncpy(m_state.ipAddress, m_wifiMgr.ipAddress(), sizeof(m_state.ipAddress) - 1U);

    // 주기적 원격 측정 데이터(Telemetry) 송신 처리
    if (m_awsClient.isConnected() && (now - m_lastTelemetryMs >= kTelemetryMs)) {
        char json[1536];
        if (cloud::TelemetryBuilder::build(m_state, m_batch, INCUBATOR_DEVICE_ID, json, sizeof(json)) > 0U) {
            m_awsClient.publishTelemetry(json);
        }
        m_lastTelemetryMs = now;
    }

    // 주기적 디바이스 헬스체크 데이터 송신 처리
    if (m_awsClient.isConnected() && (now - m_lastHealthMs >= kHealthMs)) {
        char json[512];
        if (cloud::TelemetryBuilder::buildHealth(m_state, json, sizeof(json)) > 0U) {
            m_awsClient.publishHealth(json);
        }
        m_lastHealthMs = now;
    }
#endif
}

} // namespace incubator