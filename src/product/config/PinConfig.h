#pragma once
#include <cstdint>

// namespace incubator::config
// {
//     struct Pin
//     {
//         // ── I2C (AHT20) ──────────────────────────────
//         static constexpr int I2C_SDA       = 8;
//         static constexpr int I2C_SCL       = 9;

//         // ── SPI (ST7789 TFT) ─────────────────────────
//         static constexpr int TFT_CS        = 10;
//         static constexpr int TFT_DC        = 13;
//         static constexpr int TFT_RST       = 14;
//         static constexpr int TFT_SCLK      = 12;
//         static constexpr int TFT_MOSI      = 11;

//         // ── 디지털 출력 ──────────────────────────────
//         static constexpr int SSR_HEATER     = 19;   // HIGH=ON
//         static constexpr int SSR_HUMIDIFIER = 21;   // HIGH=ON
//         static constexpr int RELAY_TURNER   = 20;   // HIGH=ON
//         // GPIO32 caused early WDT resets on the current ESP32-S3 module.
//         // Keep disabled until the actual buzzer pin is verified.
//         static constexpr int BUZZER         = -1;   // HIGH=ON (Active Buzzer)

//         // ── PWM 팬 ───────────────────────────────────
//         // Disabled until the actual fan PWM pin is verified on hardware.
//         static constexpr int FAN_PWM        = -1;
//         static constexpr int FAN_PWM_CH     = 0;    // LEDC 채널 0

//         // ── EC11 로터리 엔코더 ────────────────────────
//         static constexpr int ENC_A          = 5;   // CLK — ISR
//         static constexpr int ENC_B          = 6;   // DT  — ISR
//         static constexpr int ENC_BTN        = 4;   // SW  — 내부 풀업, Active LOW
//     };
// }
