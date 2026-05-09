#include "ui/ProvisioningRenderer.h"
#include "ui/UiColors.h"
#include <cstdio>
#include <cstring>

namespace incubator::ui
{
namespace
{
    static constexpr uint32_t kBlack    = 0x0000U;
    static constexpr uint32_t kWhite    = 0xFFFFU;
    static constexpr uint32_t kPanel    = 0x0841U;
    static constexpr uint32_t kPanel2   = 0x18E3U;
    static constexpr uint32_t kAccent   = 0x0679U;
    static constexpr uint32_t kOk       = 0x07E0U;
    static constexpr uint32_t kWarn     = 0xFFE0U;
    static constexpr uint32_t kDanger   = 0xF800U;

    void drawText(devices::St7789Display& d, int x, int y, const char* text,
                  uint8_t size, uint32_t fg, uint32_t bg)
    {
        d.setTextSize(size);
        d.setTextColor(fg, bg);
        d.drawText(x, y, text);
    }

    void drawPill(devices::St7789Display& d, int x, int y, int w,
                  const char* text, uint32_t color)
    {
        d.drawRect(x, y, w, 18, color);
        d.fillRect(x + 1, y + 1, w - 2, 16, kPanel);
        drawText(d, x + 7, y + 4, text, 1, Color::kText, kPanel);
    }

    void formatTime(uint32_t ms, char* out, size_t len)
    {
        uint32_t sec = ms / 1000U;
        uint32_t min = sec / 60U;
        sec %= 60U;
        std::snprintf(out, len, "%02u:%02u", static_cast<unsigned>(min), static_cast<unsigned>(sec));
    }
}

void ProvisioningRenderer::render(uint32_t nowMs)
{
    (void)nowMs;

    // Espressif ESP Provisioning 앱 QR payload.
    // 현재 Android/iOS provisioning 라이브러리는 security가 없으면 Sec2로 해석할 수 있다.
    // 펌웨어는 WIFI_PROV_SECURITY_1로 시작하므로 QR에도 security:1을 반드시 명시한다.
    char payload[224];
        std::snprintf(payload, sizeof(payload),
        "{\"ver\":\"v1\",\"name\":\"%s\",\"pop\":\"%s\",\"transport\":\"ble\",\"security\":1}",
                  m_model.provisioningName,
                  m_model.provisioningPop);

    m_display.fillScreen(Color::kBg);

    // Header
    m_display.fillRect(0, 0, 320, 28, kBlack);
    drawText(m_display, 10, 7, "BLE WiFi Setup", 1, Color::kText, kBlack);

    uint32_t stateColor = kAccent;
    const char* state = "SCAN";
    if (m_model.provisioningSucceeded) { state = "DONE"; stateColor = kOk; }
    else if (m_model.provisioningFailed) { state = "FAIL"; stateColor = kDanger; }
    drawPill(m_display, 254, 5, 56, state, stateColor);

    // QR scan zone: 최대한 크게, 순수 흰 배경 + quiet zone 확보
    m_display.fillRect(8, 36, 196, 196, kWhite);
    m_display.drawRect(6, 34, 200, 200, kAccent);
    m_display.drawRect(5, 33, 202, 202, kPanel2);
    m_display.drawQrCode(payload, 14, 42, 184, 10, true);

    // Right info panel
    m_display.fillRect(214, 36, 98, 196, kPanel);
    m_display.drawRect(214, 36, 98, 196, kPanel2);
    drawText(m_display, 224, 48, "ESP", 2, kAccent, kPanel);
    drawText(m_display, 224, 70, "Provisioning", 1, Color::kText, kPanel);

    drawText(m_display, 224, 96, "1. 앱 실행", 1, Color::kTextDim, kPanel);
    drawText(m_display, 224, 114, "2. QR 스캔", 1, Color::kTextDim, kPanel);
    drawText(m_display, 224, 132, "3. WiFi 선택", 1, Color::kTextDim, kPanel);

    m_display.drawLine(222, 154, 304, 154, kPanel2);
    drawText(m_display, 224, 164, "NAME", 1, kAccent, kPanel);
    drawText(m_display, 224, 178, m_model.provisioningName, 1, Color::kText, kPanel);

    drawText(m_display, 224, 198, "POP", 1, kAccent, kPanel);
    drawText(m_display, 224, 212, m_model.provisioningPop, 1, Color::kText, kPanel);
    drawText(m_display, 276, 212, "S1", 1, kOk, kPanel);

    // Bottom status strip
    m_display.fillRect(0, 232, 320, 8, kBlack);

    char remaining[16];
    formatTime(m_model.provisioningRemainingMs, remaining, sizeof(remaining));
    char msg[64];
    std::snprintf(msg, sizeof(msg), "%s  %s", m_model.provisioningMessage, remaining);
    m_display.fillRect(0, 214, 208, 18, Color::kBg);
    drawText(m_display, 12, 218, msg, 1,
             m_model.provisioningFailed ? kWarn : Color::kTextDim,
             Color::kBg);
}

} // namespace incubator::ui
