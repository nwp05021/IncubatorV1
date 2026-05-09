너는 10년차 ESP32 / IoT 펌웨어 아키텍트다.
Incubator_Developer_Reference.md 문서를 먼저 읽고나서 작업을 진행한다.

하드웨어:
- MCU: ESP32-S3
- Display: ST7789 240×320, GMT020-02-7P 2.0인치 TFT SPI — 7핀 제품, 별도 BL(백라이트) 핀 없음
- 실제 구현은 320×240(landscape)으로 진행한다.
- Aht20 온도습도 센서
- EC11 로터리 디코더 SW

개발환경:
- platformio 개발환경
- platform = espidf
- arduino as component 방식으로 arduino 는 효율성을 위한 불가피한 경우에만 함수처럼 사용

작업 순서:

1. Incubator_Developer_Reference.md 문서를 먼저 읽는다.
2. IncubatorV1-0.3.0.zip 소스를 파악한다.
- 소스는 기본 구조는 완성하였으나 실제 디바이스 연결은 시작 단계이다. Display 만 연결된 상태다.
- 단일진실 원칙을 준수한다.
3. 한글 사용 가능하도록 할 것.
- 기본 폰트는 16 정도가 적당할 것 같고
- 메인 화면의 숫자는 5 ~ 6 정도의 큰 폰트 사용
- 한글을 사용할 수 없으면 아무 의미 없음 
4. include/config/PinConfig.h 를 디바이스 설정에 대한 단일 진실로 한다.
곧 Fan 을 추가하고 테스트할 예정이라 독립된 설정 환경이 필요하다.
5. EC11 인코더는 아래 소스를 기반으로 구현한다.

#include "devices/Ec11Encoder.h"
#include <esp_log.h>
#include <driver/gpio.h>

namespace incubator::devices
{

static const int8_t kTable[4][4] = {
    { 0, -1,  1,  0},
    { 1,  0,  0, -1},
    {-1,  0,  0,  1},
    { 0,  1, -1,  0}
};

bool Ec11Encoder::init()
{
    gpio_config_t cfg = {};
    cfg.mode = GPIO_MODE_INPUT;
    cfg.pin_bit_mask = (1ULL << m_pinA) | (1ULL << m_pinB);
    cfg.pull_up_en = GPIO_PULLUP_ENABLE;
    cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cfg.intr_type = GPIO_INTR_DISABLE;
    if (gpio_config(&cfg) != ESP_OK) {
        ESP_LOGE("Ec11Encoder", "gpio_config failed for encoder pins");
        return false;
    }

    gpio_config_t btn_cfg = {};
    btn_cfg.mode = GPIO_MODE_INPUT;
    btn_cfg.pin_bit_mask = 1ULL << m_pinBtn;
    btn_cfg.pull_up_en = GPIO_PULLUP_ENABLE;
    btn_cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    btn_cfg.intr_type = GPIO_INTR_DISABLE;
    if (gpio_config(&btn_cfg) != ESP_OK) {
        ESP_LOGE("Ec11Encoder", "gpio_config failed for button");
        return false;
    }

    int a = gpio_get_level(static_cast<gpio_num_t>(m_pinA));
    int b = gpio_get_level(static_cast<gpio_num_t>(m_pinB));
    m_lastState = (a << 1) | b;
    m_detentState = m_lastState;
    return true;
}

int Ec11Encoder::consumeDelta()
{
    int d = m_delta;
    m_delta = 0;
    return d;
}

void Ec11Encoder::tick(uint32_t nowMs)
{
    pollEncoder(nowMs);

    bool raw = gpio_get_level(static_cast<gpio_num_t>(m_pinBtn)) == 0;
    if (raw != m_btnRaw) {
        if (nowMs - m_lastBounceMs >= kDebounceMs) {
            m_lastBounceMs = nowMs;
            m_btnRaw = raw;
            if (raw) {
                m_btnDownMs = nowMs;
                m_waitRelease = true;
            } else if (m_waitRelease) {
                uint32_t pressedMs = nowMs - m_btnDownMs;
                if (pressedMs >= kLongPressMs) {
                    m_btnLongPress = true;
                } else {
                    m_btnPressed = true;
                }
                m_waitRelease = false;
            }
        }
    }
}

bool Ec11Encoder::wasPressed()
{
    if (!m_btnPressed) return false;
    m_btnPressed = false;
    return true;
}

bool Ec11Encoder::wasLongPressed()
{
    if (!m_btnLongPress) return false;
    m_btnLongPress = false;
    return true;
}

void Ec11Encoder::pollEncoder(uint32_t nowMs)
{
    int a = gpio_get_level(static_cast<gpio_num_t>(m_pinA));
    int b = gpio_get_level(static_cast<gpio_num_t>(m_pinB));
    int state = (a << 1) | b;
    if (state == m_lastState) return;
    if (nowMs - m_lastEncoderMs < kEncoderDebounceMs) return;

    int prev = m_lastState;
    int8_t movement = kTable[prev][state];
    m_lastState = state;
    m_lastEncoderMs = nowMs;

    if (movement == 0) {
        m_stepAccum = 0;
        return;
    }

    m_stepAccum += movement;
    if (state != m_detentState) {
        if (m_stepAccum > 8 || m_stepAccum < -8) {
            m_stepAccum = 0;
        }
        return;
    }

    if (nowMs - m_lastEmitMs >= kEncoderEmitGapMs) {
        if (m_stepAccum >= 3) {
            ++m_delta;
            m_lastEmitMs = nowMs;
        } else if (m_stepAccum <= -3) {
            --m_delta;
            m_lastEmitMs = nowMs;
        }
    }
    m_stepAccum = 0;
}

} // namespace incubator::devices

6. AHT20 센서가 실제 작동하도록 구현한다.
- I2C 를 사용하는 디바이스들은 여러번 죽는 경험을 하였다. 공유 방식으로 구현한다.
- espidf 기준으로 구현한다.  

7. 여기까지 하여 기본 디바이스들이 작동하면 DDU-01.md 문서를 읽고 UI 구현을 한다.

중요:
- 5번 까지 먼저 구현되어야 내가 테스트를 해 볼 수 있다.
- 구현은 espidf 가 원칙이며 Arduino 는 불가피한 경우에 한해 사용한다.
- 향 후 OTA 까지 구현할 예정이므로 espidf 우선 적용한다.
- 불가피한 경우 arduino as components 방식으로 아두이노 사용 가능하다.

즉 이번 작업의 목표는 DDU-01.md 문서 내용 까지이다.
중요: 
- 꼭 IncubatorV1-0.3.0.zip 소스의 내용을 파악한 후 DDU-01.md 요구 조건을 이해한 후 진행하길 바란다. 가능하면 한, 두번의 턴에 일괄 처리해주길 바란다. 초보자도 쉽게 처리할 수 있는 내용이니 나를 실망시키지 않기를 바란다.
- 소스 확인 없는 가상 코드는 절대 금물이다.
