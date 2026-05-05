# DDU-003 — Device Layer
> **Document ID**: DDU-003  
> **Version**: 1.0  
> **상위 문서**: INC-IMPL-001 §9  
> **의존 DDU**: DDU-001  
> **Codex 작업 시간 예상**: 20~30분

---

## 작업 목표

이 DDU 완료 후:
- I2C 버스가 ESP-IDF i2c_master로 초기화된다 (Arduino Wire 완전 배제)
- AHT20 온도/습도 측정이 non-blocking으로 동작한다
- SSR/Relay/Buzzer GPIO가 정확히 제어된다
- PWM 팬이 0~100% duty 제어된다
- EC11 엔코더가 루프 폴링 기반 Gray code delta + 버튼 폴링으로 동작한다
- ST7789 TFT가 LovyanGFX로 초기화된다

---

## 1. 생성할 파일 목록

```
include/devices/I2cBus.h
src/devices/I2cBus.cpp

include/devices/Aht20Driver.h
src/devices/Aht20Driver.cpp

include/devices/GpioOutput.h
src/devices/GpioOutput.cpp

include/devices/PwmFan.h
src/devices/PwmFan.cpp

include/devices/Ec11Encoder.h
src/devices/Ec11Encoder.cpp

include/devices/LgfxConfig.h       ← LovyanGFX 설정 클래스 (헤더 전용)
include/devices/St7789Display.h
src/devices/St7789Display.cpp
```

---

## 2. I2cBus

### 2.1 헤더

```cpp
// include/devices/I2cBus.h
// ★ Arduino Wire 직접 호출 금지 — 초기화 경쟁으로 재부팅 원인.
//   모든 I2C 통신은 이 클래스를 통한다.
#pragma once
#include <driver/i2c_master.h>
#include <cstdint>
#include <cstddef>

namespace incubator::devices
{
    class I2cBus
    {
    public:
        static constexpr uint8_t  kMaxDevices = 8;
        static constexpr uint32_t kTimeoutMs  = 50U;

        bool init(int sdaPin, int sclPin, uint32_t freqHz = 400000U);
        bool isInitialized() const { return m_busHandle != nullptr; }

        bool write   (uint8_t addr, const uint8_t* data, size_t len);
        bool read    (uint8_t addr, uint8_t* buf,        size_t len);
        bool writeRead(uint8_t addr,
                       const uint8_t* txData, size_t txLen,
                       uint8_t*       rxBuf,  size_t rxLen);
        bool isReady (uint8_t addr);   // ACK 확인

    private:
        i2c_master_bus_handle_t m_busHandle = nullptr;

        struct DevCache {
            uint8_t                 addr   = 0;
            i2c_master_dev_handle_t handle = nullptr;
        };
        DevCache m_devCache[kMaxDevices] = {};
        uint8_t  m_devCount = 0;

        i2c_master_dev_handle_t getOrAddDevice(uint8_t addr);
    };
}
```

### 2.2 구현 요점

```cpp
// src/devices/I2cBus.cpp
// ESP-IDF 5.x i2c_master API 사용.
// 핵심 패턴:
//   init()       → i2c_master_bus_create()
//   getOrAdd()   → i2c_master_bus_add_device() (캐시, 중복 방지)
//   write()      → i2c_master_transmit()
//   read()       → i2c_master_receive()
//   writeRead()  → i2c_master_transmit_receive()
//
// 모든 함수는 kTimeoutMs pdMS_TO_TICKS로 변환해서 타임아웃 적용.
// 오류 시 ESP_LOGE 로그 후 false 반환 (크래시 없음).
```

> **Codex에게**: I2cBus.cpp 전체를 위 요점에 따라 구현한다.  
> ESP-IDF `i2c_master.h` API를 사용하며 Arduino `Wire.h` include 금지.

---

## 3. Aht20Driver

### 3.1 헤더

```cpp
// include/devices/Aht20Driver.h
#pragma once
#include "I2cBus.h"
#include <cstdint>

namespace incubator::devices
{
    struct Aht20Reading
    {
        float    tempC       = 0.0f;
        float    humidityPct = 0.0f;
        bool     valid       = false;
        uint32_t timestampMs = 0U;
    };

    class Aht20Driver
    {
    public:
        static constexpr uint8_t  kAddress      = 0x38U;
        static constexpr uint32_t kMeasureDelayMs = 80U;   // AHT20 변환 대기

        explicit Aht20Driver(I2cBus& bus) : m_bus(bus) {}

        bool init();

        // 2단계 비동기 측정
        bool triggerMeasurement();   // 측정 요청 (I2C write)
        bool fetchResult();          // 결과 읽기 (80ms 후 호출)

        float    getCachedTemp()      const { return m_cached.tempC; }
        float    getCachedHumi()      const { return m_cached.humidityPct; }
        bool     isCacheValid()       const { return m_cached.valid; }
        uint32_t getCacheTimestamp()  const { return m_cached.timestampMs; }
        bool     isConnected()        const { return m_ok; }

        bool reinitialize();   // 센서 오류 후 재초기화

    private:
        I2cBus&      m_bus;
        Aht20Reading m_cached         = {};
        bool         m_ok             = false;
        bool         m_measurePending = false;
        uint32_t     m_triggerMs      = 0;
    };
}
```

### 3.2 AHT20 통신 프로토콜 (구현 필수 참조)

```
// init():
//   1. Write cmd[3] = {0xBE, 0x08, 0x00}  ← 소프트 리셋/초기화
//   2. delay 10ms
//   3. Write cmd[3] = {0xAC, 0x33, 0x00}  ← 첫 측정 트리거
//   4. m_ok = (I2C write 성공)

// triggerMeasurement():
//   Write cmd[3] = {0xAC, 0x33, 0x00}
//   m_measurePending = true
//   m_triggerMs = millis()

// fetchResult(): ← triggerMs + kMeasureDelayMs 이후에만 유효
//   Read buf[6]
//   if (buf[0] & 0x80) → 아직 변환 중, false 반환
//   raw_humi = ((uint32_t)(buf[1])<<12) | ((uint32_t)(buf[2])<<4) | (buf[3]>>4)
//   raw_temp = (((uint32_t)(buf[3] & 0x0F))<<16) | ((uint32_t)(buf[4])<<8) | buf[5]
//   m_cached.humidityPct = (float)raw_humi / 1048576.0f * 100.0f
//   m_cached.tempC       = (float)raw_temp / 1048576.0f * 200.0f - 50.0f
//   m_cached.valid       = true
//   m_cached.timestampMs = millis()
//   m_measurePending     = false
```

---

## 4. GpioOutput

```cpp
// include/devices/GpioOutput.h
// SSR / Relay / Buzzer 공통 GPIO 출력 래퍼.
#pragma once
#include <driver/gpio.h>
#include <cstdint>

namespace incubator::devices
{
    class GpioOutput
    {
    public:
        // invertLogic=true: HIGH→OFF, LOW→ON (일부 relay 보드)
        explicit GpioOutput(gpio_num_t pin, bool invertLogic = false)
            : m_pin(pin), m_invert(invertLogic) {}

        bool init();
        void set(bool on);
        bool isOn()  const { return m_state; }
        void on()          { set(true); }
        void off()         { set(false); }
        void toggle()      { set(!m_state); }

    private:
        gpio_num_t m_pin;
        bool       m_invert = false;
        bool       m_state  = false;
    };
}

// src/devices/GpioOutput.cpp 구현 요점:
// init() : gpio_config_t → GPIO_MODE_OUTPUT, gpio_config()
// set()  : gpio_set_level(m_pin, (on ^ m_invert) ? 1 : 0)
//          m_state = on
```

---

## 5. PwmFan

```cpp
// include/devices/PwmFan.h
#pragma once
#include <driver/ledc.h>
#include <cstdint>

namespace incubator::devices
{
    class PwmFan
    {
    public:
        PwmFan(int pin, ledc_channel_t channel, uint32_t freqHz = 25000U)
            : m_pin(pin), m_channel(channel), m_freq(freqHz) {}

        bool    init();
        void    setDuty(uint8_t percent);   // 0~100
        uint8_t getDuty() const { return m_duty; }
        void    stop()          { setDuty(0); }

    private:
        int            m_pin;
        ledc_channel_t m_channel;
        uint32_t       m_freq;
        uint8_t        m_duty   = 0;

        static constexpr ledc_timer_t   kTimer     = LEDC_TIMER_0;
        static constexpr ledc_mode_t    kMode      = LEDC_LOW_SPEED_MODE;
        static constexpr ledc_timer_bit_t kDutyBit = LEDC_TIMER_8_BIT; // 0~255
    };
}

// src/devices/PwmFan.cpp 구현 요점:
// init():
//   ledc_timer_config_t → ledc_timer_config()
//   ledc_channel_config_t → ledc_channel_config()
// setDuty(pct):
//   duty = (uint32_t)pct * 255 / 100
//   ledc_set_duty(kMode, m_channel, duty)
//   ledc_update_duty(kMode, m_channel)
//   m_duty = pct
```

---

## 6. Ec11Encoder

### 6.1 헤더

```cpp
// include/devices/Ec11Encoder.h
// ★ 안정성 규칙: ISR 사용 금지. loop tick에서 Gray code를 폴링한다.
//   printf / Serial / malloc / EventBus 절대 금지.
#pragma once
#include <driver/gpio.h>
#include <cstdint>

namespace incubator::devices
{
    class Ec11Encoder
    {
    public:
        Ec11Encoder(int pinA, int pinB, int pinBtn)
            : m_pinA(pinA), m_pinB(pinB), m_pinBtn(pinBtn) {}

        bool init();

        // Tick 내에서 호출 — ISR delta 소비 (원자적)
        int  consumeDelta();

        // 버튼 debounce tick (loop에서 매 호출)
        void tick(uint32_t nowMs);

        // 버튼 상태 (소비형 — 한 번 읽으면 false로 리셋)
        bool wasPressed();     // 짧게 누름 (< kLongPressMs)
        bool wasLongPressed(); // 길게 누름 (>= kLongPressMs)

    private:
        int m_pinA, m_pinB, m_pinBtn;

        volatile int m_delta      = 0;
        volatile int m_lastState  = 0;  // Gray code 이전 상태

        // 버튼
        bool     m_btnRaw        = false;
        bool     m_btnPressed    = false;
        bool     m_btnLongPress  = false;
        bool     m_waitRelease   = false;
        uint32_t m_btnDownMs     = 0;
        uint32_t m_lastBounceMs  = 0;

        static constexpr uint32_t kDebounceMs  = 30U;
        static constexpr uint32_t kLongPressMs = 1500U;

        static void IRAM_ATTR isrHandler(void* arg);
    };
}
```

### 6.2 ISR 구현 요점

```cpp
// ISR — IRAM_ATTR 필수
void IRAM_ATTR Ec11Encoder::isrHandler(void* arg)
{
    auto* self = static_cast<Ec11Encoder*>(arg);
    int a = gpio_get_level((gpio_num_t)self->m_pinA);
    int b = gpio_get_level((gpio_num_t)self->m_pinB);
    int state = (a << 1) | b;  // Gray code 2비트

    // 4-step Gray code 전이 테이블
    // CW:  00→01→11→10→00
    // CCW: 00→10→11→01→00
    static const int8_t kTable[4][4] = {
        { 0, -1,  1,  0},
        { 1,  0,  0, -1},
        {-1,  0,  0,  1},
        { 0,  1, -1,  0}
    };
    int prev = self->m_lastState;
    self->m_delta += kTable[prev][state];
    self->m_lastState = state;
}
```

### 6.3 init() 요점

```cpp
// init():
//   1. GPIO_MODE_INPUT, GPIO_PULLUP_ENABLE, GPIO_INTR_ANYEDGE 로 ENC_A, ENC_B 설정
//   2. GPIO_MODE_INPUT, GPIO_PULLUP_ENABLE, GPIO_INTR_DISABLE 로 ENC_BTN 설정
//   3. gpio_install_isr_service(0)
//   4. gpio_isr_handler_add(ENC_A, isrHandler, this)
//   5. gpio_isr_handler_add(ENC_B, isrHandler, this)
//   m_lastState 초기화

// consumeDelta():
//   portDISABLE_INTERRUPTS()
//   int d = m_delta;  m_delta = 0;
//   portENABLE_INTERRUPTS()
//   return d;
```

---

## 7. LgfxConfig.h + St7789Display

### 7.1 LgfxConfig.h (헤더 전용)

```cpp
// include/devices/LgfxConfig.h
// LovyanGFX 하드웨어 설정.
// GMT020-02-7P 2.0인치 ST7789, 7핀, BL 핀 없음.
#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "config/PinConfig.h"

class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ST7789  _panel;
    lgfx::Bus_SPI       _bus;

public:
    LGFX()
    {
        // SPI 버스 설정
        {
            auto cfg      = _bus.config();
            cfg.spi_host  = SPI2_HOST;
            cfg.spi_clock = 40000000;
            cfg.pin_sclk  = incubator::config::Pin::TFT_SCLK;
            cfg.pin_mosi  = incubator::config::Pin::TFT_MOSI;
            cfg.pin_miso  = -1;
            cfg.pin_dc    = incubator::config::Pin::TFT_DC;
            _bus.config(cfg);
            _panel.setBus(&_bus);
        }
        // 패널 설정
        {
            auto cfg       = _panel.config();
            cfg.pin_cs     = incubator::config::Pin::TFT_CS;
            cfg.pin_rst    = incubator::config::Pin::TFT_RST;
            cfg.pin_busy   = -1;
            cfg.panel_width  = 240;
            cfg.panel_height = 320;
            cfg.offset_rotation = 1;
            cfg.invert = true;         // ST7789 색상 반전 필요
            _panel.config(cfg);
        }
        setPanel(&_panel);
    }
};
```

### 7.2 St7789Display 헤더

```cpp
// include/devices/St7789Display.h
#pragma once
#include "LgfxConfig.h"
#include <cstdint>

namespace incubator::devices
{
    class St7789Display
    {
    public:
        bool init();

        // 프레임 버퍼 제어 (LovyanGFX DMA sprite)
        void beginFrame();
        void endFrame();

        // 기본 그리기
        void fillScreen(uint32_t color);
        void fillRect(int x, int y, int w, int h, uint32_t color);
        void drawLine(int x1, int y1, int x2, int y2, uint32_t color);
        void drawRect(int x, int y, int w, int h, uint32_t color);

        // 텍스트
        void drawText(int x, int y, const char* text);
        void setTextColor(uint32_t fg, uint32_t bg = 0x0000U);
        void setTextSize(uint8_t size);   // 1~7

        // LovyanGFX 직접 접근 (MainUiRenderer 전용)
        LGFX& gfx() { return m_gfx; }

    private:
        LGFX         m_gfx;
        bool         m_initialized = false;
    };
}

// src/devices/St7789Display.cpp 구현 요점:
// init():
//   m_gfx.init()
//   m_gfx.setRotation(1)       ← 320x240 landscape, right-rotated viewing direction
//   m_gfx.fillScreen(TFT_BLACK)
//   m_initialized = true
//
// beginFrame() / endFrame():
//   LovyanGFX startWrite() / endWrite() 또는 DMA sprite push
//   스프라이트 사용 시: m_sprite.createSprite(320, 240) in init()
//                      beginFrame = m_sprite.fillScreen(BLACK)
//                      endFrame   = m_sprite.pushSprite(0, 0)
//
// drawText() : m_gfx.setCursor(x, y); m_gfx.print(text)
```

---

## 완료 기준 (Acceptance Criteria)

| # | 항목 | 기준 |
|---|---|---|
| AC-1 | I2C 초기화 | `i2c.init(SDA, SCL)` 성공, Serial에 "I2C OK" |
| AC-2 | AHT20 측정 | 온도 15~45°C 범위, 습도 0~100% 범위 내 값 출력 |
| AC-3 | AHT20 비동기 | `triggerMeasurement()` → 80ms 후 `fetchResult()` 패턴 준수 |
| AC-4 | GPIO ON/OFF | Heater SSR: Serial "H ON" 출력 시 실제 LED/SSR 점등 확인 |
| AC-5 | PWM Fan | duty 0, 50, 100 설정 시 팬 속도 변화 확인 |
| AC-6 | EC11 delta | CW 3클릭 → consumeDelta() == 3 |
| AC-7 | EC11 버튼 | 짧게 누름 → wasPressed()==true, 이후 false |
| AC-8 | EC11 LongPress | 1.5초 누름 → wasLongPressed()==true |
| AC-9 | EC11 안정성 | ENC_A/ENC_B ISR 미사용, 회전 시 WDT 리셋 없음 |
| AC-10 | TFT 초기화 | 화면에 검은 배경 표시, drawText 동작 확인 |
| AC-11 | Wire 금지 | `Wire.h` include 없음 (전체 파일 검색) |
