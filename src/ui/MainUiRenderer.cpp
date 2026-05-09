#include "ui/MainUiRenderer.h"
#include "ui/UiLayout.h"
#include "ui/UiColors.h"
#include <cmath>
#include <cstdio>
#include <cstring>

namespace incubator::ui
{
namespace
{
    static constexpr uint32_t kPanel      = 0x0841U;
    static constexpr uint32_t kPanelSoft  = 0x18E3U;
    static constexpr uint32_t kOk         = 0x07E0U;
    static constexpr uint32_t kWarn       = 0xFFE0U;
    static constexpr uint32_t kDanger     = 0xF800U;
    static constexpr uint32_t kTeal       = 0x0679U;
    static constexpr uint8_t  kMenuCount  = 8;

    void hashBytes(uint32_t& hash, const void* data, size_t len)
    {
        const auto* p = static_cast<const uint8_t*>(data);
        while (len-- > 0U) {
            hash ^= *p++;
            hash *= 16777619U;
        }
    }

    template <typename T>
    void hashValue(uint32_t& hash, const T& value)
    {
        hashBytes(hash, &value, sizeof(T));
    }

    void hashString(uint32_t& hash, const char* text)
    {
        if (!text) {
            uint8_t zero = 0;
            hashValue(hash, zero);
            return;
        }
        while (*text) {
            uint8_t c = static_cast<uint8_t>(*text++);
            hashValue(hash, c);
        }
        uint8_t zero = 0;
        hashValue(hash, zero);
    }

    int16_t q10(float value)
    {
        return static_cast<int16_t>(std::lroundf(value * 10.0f));
    }

    int16_t q1(float value)
    {
        return static_cast<int16_t>(std::lroundf(value));
    }

    uint32_t visibleHash(const UiModel& model)
    {
        uint32_t hash = 2166136261U;
        const uint32_t clockMinute = model.uptimeMs / 60000U;

        hashValue(hash, model.screen);
        hashValue(hash, model.activePage);
        hashValue(hash, clockMinute);
        hashValue(hash, model.currentDay);
        hashValue(hash, model.totalDays);
        hashValue(hash, model.progressPct);
        hashValue(hash, model.cloudConnected);
        hashValue(hash, model.wifiConfigured);
        hashValue(hash, model.tempAlarm);
        hashValue(hash, model.humiAlarm);
        hashValue(hash, model.tempSensorFault);
        hashValue(hash, model.humiSensorFault);
        hashValue(hash, model.lockdownActive);
        hashValue(hash, model.safeMode);

        hashValue(hash, q10(model.displayTempC));
        hashValue(hash, q1(model.displayHumidPct));
        hashValue(hash, q10(model.targetTempC));
        hashValue(hash, q1(model.targetHumidPct));
        hashValue(hash, model.heaterOn);
        hashValue(hash, model.humidifierOn);
        hashValue(hash, model.turnerOn);
        hashValue(hash, model.fanOn);
        hashValue(hash, model.turningEnabled);
        hashValue(hash, model.nextTurningInMin);

        hashValue(hash, model.menuCursor);
        hashValue(hash, model.manualCursor);
        hashValue(hash, model.confirmCursor);
        hashValue(hash, model.presetCursor);
        hashValue(hash, model.presetConfirm);
        hashValue(hash, model.editMode);
        hashValue(hash, model.fieldCursor);
        hashValue(hash, model.editBatchYear);
        hashValue(hash, model.editBatchMonth);
        hashValue(hash, model.editBatchDay);
        hashValue(hash, model.editDay);
        hashValue(hash, q10(model.editTempC));
        hashValue(hash, q1(model.editHumidPct));
        hashValue(hash, model.editTurning);
        hashValue(hash, model.editIntervalMin);
        hashValue(hash, model.activeEditField);
        hashValue(hash, model.factoryProgressPct);
        hashValue(hash, model.factoryReady);
        hashString(hash, model.actionMessage);

        hashValue(hash, model.provisioningActive);
        hashValue(hash, model.provisioningSucceeded);
        hashValue(hash, model.provisioningFailed);
        hashValue(hash, model.provisioningRemainingMs / 1000U);
        hashString(hash, model.provisioningName);
        hashString(hash, model.provisioningPop);
        hashString(hash, model.provisioningMessage);

        if (model.screen == UiScreen::System) {
            hashValue(hash, model.uptimeMs / 1000U);
            hashValue(hash, model.bootCount);
            hashValue(hash, model.batchActive);
            hashString(hash, model.ipAddress);
        }

        return hash;
    }

    const char* screenTitle(const UiModel& model)
    {
        if (model.screen == UiScreen::Menu) return "메뉴";
        if (model.screen == UiScreen::StartDate) return "부화 시작일";
        if (model.screen == UiScreen::Preset) return "프리셋 선택";
        if (model.screen == UiScreen::PlanList || model.screen == UiScreen::PlanEdit) return "부화 스케줄";
        if (model.screen == UiScreen::Manual) return "수동 테스트";
        if (model.screen == UiScreen::System) return "SYSTEM";
        if (model.screen == UiScreen::WifiReset) return "WiFi 정보 리셋";
        if (model.screen == UiScreen::RebootConfirm) return "시스템 재부팅";
        if (model.screen == UiScreen::FactoryReset) return "공장 초기화";
        switch (model.activePage) {
            case 1: return "상태 정보";
            case 2: return "도움말";
            default: return "부화기";
        }
    }

    bool hasUtf8(const char* text)
    {
        if (!text) return false;
        while (*text) {
            if ((static_cast<unsigned char>(*text) & 0x80U) != 0U) return true;
            ++text;
        }
        return false;
    }

    void formatClock(uint32_t uptimeMs, char* out, size_t len)
    {
        uint32_t minutes = uptimeMs / 60000U;
        uint32_t hours = (minutes / 60U) % 24U;
        minutes %= 60U;
        std::snprintf(out, len, "%02u:%02u", hours, minutes);
    }

    const char* presetName(uint8_t i)
    {
        switch (i) {
            case 0: return "닭";
            case 1: return "오리";
            case 2: return "메추리";
            case 3: return "거위";
            default: return "닭";
        }
    }
}

void MainUiRenderer::render(uint32_t nowMs)
{
    const uint32_t hash = visibleHash(m_model);
    if (m_hasRendered && hash == m_lastVisibleHash) return;
    if (m_hasRendered && nowMs - m_lastRenderMs < 50U) return;

    m_lastRenderMs = nowMs;
    m_lastVisibleHash = hash;
    m_hasRendered = true;

    m_display.beginFrame();
    if (m_model.provisioningActive) {
        m_provisioningRenderer.render(nowMs);
        m_display.endFrame();
        return;
    }

    m_display.fillScreen(Color::kBg);
    drawStatusBar();
    renderHeader();

    switch (m_model.screen) {
        case UiScreen::Main:
            if (m_model.activePage == 0) renderPage0();
            else if (m_model.activePage == 1) renderPage1();
            else renderHelp();
            break;
        case UiScreen::Menu: renderMenu(); break;
        case UiScreen::StartDate: renderStartDate(); break;
        case UiScreen::Preset: renderPreset(); break;
        case UiScreen::PlanList: renderPlanList(); break;
        case UiScreen::PlanEdit: renderPlanEdit(); break;
        case UiScreen::Manual: renderManual(); break;
        case UiScreen::WifiReset:
            renderConfirm("WiFi 정보 리셋", "저장된 WiFi 인증정보를 삭제합니다.", "삭제 후 BLE 설정이 필요합니다.");
            break;
        case UiScreen::RebootConfirm:
            renderConfirm("시스템 재부팅", "장치를 다시 시작합니다.", "진행 중 제어가 잠시 중단됩니다.");
            break;
        case UiScreen::FactoryReset: renderFactoryReset(); break;
        case UiScreen::System: renderPage4(); break;
    }

    renderFooter();
    if (m_model.safeMode) {
        m_display.fillRect(72, 102, 176, 34, kDanger);
        m_display.setTextSize(2);
        m_display.setTextColor(Color::kText, kDanger);
        m_display.drawText(104, 112, "SAFE MODE");
    }
    m_display.endFrame();
}

void MainUiRenderer::drawStatusBar()
{
    char buffer[32];
    m_display.fillRect(0, 0, Layout::kScreenW, 24, 0x0000U);
    m_display.setTextSize(2);
    m_display.setTextColor(Color::kText, 0x0000U);
    formatClock(m_model.uptimeMs, buffer, sizeof(buffer));
    m_display.drawText(8, 5, buffer);

    std::snprintf(buffer, sizeof(buffer), "Day %u/%u", m_model.currentDay, m_model.totalDays);
    m_display.setTextSize(1);
    m_display.setTextColor(Color::kTextDim, 0x0000U);
    m_display.drawText(78, 8, buffer);

    m_display.setTextColor(m_model.cloudConnected ? kOk : Color::kTextDim, 0x0000U);
    drawSignalBars(230, 6, m_model.cloudConnected || m_model.wifiConfigured);
    m_display.drawRect(251, 7, 10, 10, (m_model.tempAlarm || m_model.humiAlarm || m_model.tempSensorFault || m_model.humiSensorFault) ? kDanger : kPanelSoft);
    if (m_model.lockdownActive) {
        m_display.setTextColor(kWarn, 0x0000U);
        m_display.drawText(264, 8, "LOCK");
    } else {
        std::snprintf(buffer, sizeof(buffer), "%u%%", m_model.progressPct);
        m_display.setTextColor(Color::kTextDim, 0x0000U);
        m_display.drawText(284, 8, buffer);
    }
}

void MainUiRenderer::renderHeader()
{
    m_display.fillRect(0, 24, Layout::kScreenW, 26, Color::kHeader);
    m_display.setTextSize(1);
    m_display.setTextColor(Color::kText, Color::kHeader);
    m_display.drawText(12, 29, screenTitle(m_model));
    m_display.setTextColor(Color::kTextDim, Color::kHeader);
    m_display.drawText(244, 33, "2026-05-05");
    m_display.drawLine(0, 49, Layout::kScreenW, 49, Color::kDivider);
}

void MainUiRenderer::renderFooter()
{
    m_display.drawLine(0, 215, Layout::kScreenW, 215, Color::kDivider);
    m_display.fillRect(0, 216, Layout::kScreenW, 24, Color::kFooter);
    if (m_model.screen == UiScreen::Main) {
        drawStatusIcons();
        m_display.setTextColor(Color::kTextDim, Color::kFooter);
        m_display.drawText(204, 225, "길게: 메뉴");
    } else {
        m_display.setTextSize(1);
        m_display.setTextColor(Color::kTextDim, Color::kFooter);
        const char* hint = "회전: 이동  클릭: 선택  길게: 뒤로";
        if (m_model.screen == UiScreen::Manual) hint = "회전: 선택  클릭: 토글  길게: 종료";
        if (m_model.screen == UiScreen::PlanEdit) hint = "회전: 이동  클릭: 편집/확정";
        if (m_model.screen == UiScreen::FactoryReset) hint = "버튼 10초 유지: 실행  길게: 뒤로";
        m_display.drawText(12, 225, hint);
    }
}

void MainUiRenderer::renderPage0()
{
    char buffer[48];
    m_display.setTextColor(m_model.tempSensorFault ? kDanger : Color::kAccentTemp, Color::kBg);
    m_display.setTextSize(1);
    m_display.drawText(44, 66, "온도");
    std::snprintf(buffer, sizeof(buffer), "%.1f", m_model.displayTempC);
    m_display.drawNumberText(16, 92, buffer);
    if (m_model.tempSensorFault) {
        m_display.setTextSize(1);
        m_display.drawText(76, 150, "SENSOR");
    }
    m_display.setTextSize(2);
    m_display.drawText(112, 126, "C");

    m_display.setTextColor(m_model.humiSensorFault ? kDanger : Color::kAccentHumi, Color::kBg);
    m_display.setTextSize(1);
    m_display.drawText(208, 66, "습도");
    std::snprintf(buffer, sizeof(buffer), "%.0f", m_model.displayHumidPct);
    m_display.drawNumberText(178, 92, buffer);
    if (m_model.humiSensorFault) {
        m_display.setTextSize(1);
        m_display.drawText(218, 150, "SENSOR");
    }
    m_display.setTextSize(2);
    m_display.drawText(260, 126, "%");

    std::snprintf(buffer, sizeof(buffer), "목표 %.1fC", m_model.targetTempC);
    drawPill(28, 166, 104, buffer, Color::kAccentTemp);
    std::snprintf(buffer, sizeof(buffer), "목표 %.0f%%", m_model.targetHumidPct);
    drawPill(184, 166, 104, buffer, Color::kAccentHumi);
}

void MainUiRenderer::renderPage1()
{
    char buffer[48];
    std::snprintf(buffer, sizeof(buffer), "%.1f C", m_model.targetTempC);
    drawRow(62, "목표 온도", buffer, false, Color::kAccentTemp);
    std::snprintf(buffer, sizeof(buffer), "%.0f %%", m_model.targetHumidPct);
    drawRow(94, "목표 습도", buffer, false, Color::kAccentHumi);
    drawRow(126, "전란", m_model.lockdownActive ? "LOCK" : (m_model.turningEnabled ? "ON" : "OFF"), false,
            m_model.lockdownActive ? kWarn : (m_model.turningEnabled ? kOk : Color::kOffIcon));
    std::snprintf(buffer, sizeof(buffer), "%u min", m_model.nextTurningInMin);
    drawRow(158, "다음 전란", buffer, false, kTeal);
}

void MainUiRenderer::renderHelp()
{
    drawRow(62, "기본", "회전으로 화면 전환", false, kTeal);
    drawRow(96, "메뉴", "길게 누르기", false, Color::kTextDim);
    drawRow(130, "수동", "테스트 후 자동 복귀", false, kWarn);
    drawRow(164, "WiFi", "BLE QR 설정 지원", false, Color::kAccentHumi);
}

void MainUiRenderer::renderPage2() { renderManual(); }
void MainUiRenderer::renderPage3() { renderPlanEdit(); }

void MainUiRenderer::renderPage4()
{
    char buffer[48];
    std::snprintf(buffer, sizeof(buffer), "%u s", m_model.uptimeMs / 1000U);
    drawRow(58, "Uptime", buffer, false, kTeal);
    std::snprintf(buffer, sizeof(buffer), "%u", m_model.bootCount);
    drawRow(84, "Boot cnt", buffer, false, Color::kTextDim);
    drawRow(110, "Cloud", m_model.cloudConnected ? "ON" : "OFF", false, m_model.cloudConnected ? kOk : Color::kOffIcon);
    drawRow(136, "IP", m_model.ipAddress[0] ? m_model.ipAddress : "IP 없음", false, Color::kTextDim);
    drawRow(162, "Batch", m_model.batchActive ? "ACTIVE" : "STOP", false, m_model.batchActive ? kOk : Color::kOffIcon);
    drawRow(188, "SafeMode", m_model.safeMode ? "YES" : "NO", false, m_model.safeMode ? kDanger : kOk);
}

void MainUiRenderer::renderMenu()
{
    static constexpr const char* kNames[kMenuCount] = {
        "부화 시작일", "프리셋 선택", "일별 설정", "수동 테스트",
        "WiFi 리셋", "BLE 설정", "재부팅", "공장 초기화"
    };
    static constexpr const char* kDesc[kMenuCount] = {
        "날짜", "종 선택", "테이블", "배선 점검",
        "인증 삭제", "QR 연결", "시스템", "10초 유지"
    };

    for (uint8_t i = 0; i < kMenuCount; ++i) {
        int y = 54 + static_cast<int>(i) * 20;
        bool selected = (m_model.menuCursor == i);
        uint32_t bg = selected ? Color::kSelected : kPanel;
        m_display.fillRect(10, y, 300, 18, bg);
        m_display.drawRect(10, y, 300, 18, selected ? kTeal : kPanelSoft);
        m_display.fillRect(18, y + 5, 5, 8, selected ? kTeal : Color::kTextDim);
        m_display.setTextSize(1);
        m_display.setTextColor(Color::kText, bg);
        m_display.drawText(30, y + 3, kNames[i]);
        m_display.setTextColor(Color::kTextDim, bg);
        m_display.drawText(190, y + 3, kDesc[i]);
    }
}

void MainUiRenderer::renderStartDate()
{
    char value[24];
    std::snprintf(value, sizeof(value), "%u", m_model.editBatchYear);
    drawRow(66, "부화시작 년도", value, m_model.fieldCursor == 0, Color::kAccentTemp);
    std::snprintf(value, sizeof(value), "%u", m_model.editBatchMonth);
    drawRow(100, "부화시작 월", value, m_model.fieldCursor == 1, Color::kAccentHumi);
    std::snprintf(value, sizeof(value), "%u", m_model.editBatchDay);
    drawRow(134, "부화시작 일", value, m_model.fieldCursor == 2, kTeal);
    drawRow(176, "[저장]", "적용", m_model.fieldCursor == 3, kOk);
    drawRow(176, "[취소]", "복귀", m_model.fieldCursor == 4, Color::kOffIcon);
    if (m_model.fieldCursor == 4) {
        m_display.drawRect(160, 176, 148, 28, Color::kOffIcon);
    }
}

void MainUiRenderer::renderPreset()
{
    if (m_model.presetConfirm) {
        renderConfirm("프리셋 적용", "정말 다시 생성하겠습니까?", "현재 계획이 새 프리셋으로 바뀝니다.");
        return;
    }
    for (uint8_t i = 0; i < 4; ++i) {
        char no[8];
        std::snprintf(no, sizeof(no), "%u", i + 1U);
        drawRow(64 + i * 32, no, presetName(i), m_model.presetCursor == i, kTeal);
    }
}

void MainUiRenderer::renderPlanList()
{
    m_display.setTextSize(1);
    m_display.setTextColor(Color::kTextDim, Color::kBg);
    m_display.drawText(12, 56, "Day   Temp    Humi    Turn    Int");
    uint16_t first = (m_model.editDay > 4U) ? static_cast<uint16_t>(m_model.editDay - 3U) : 1U;
    for (uint8_t i = 0; i < 5; ++i) {
        uint16_t day = first + i;
        if (day > m_model.totalDays) break;
        int y = 72 + i * 28;
        bool sel = (day == m_model.editDay);
        uint32_t bg = sel ? Color::kSelected : Color::kBg;
        m_display.fillRect(8, y, 304, 26, bg);
        m_display.drawRect(8, y, 304, 26, sel ? kTeal : kPanelSoft);
        char row[64];
        std::snprintf(row, sizeof(row), "D%02u %.1fC %.0f%% %s %um",
                      day,
                      day == m_model.editDay ? m_model.editTempC : m_model.targetTempC,
                      day == m_model.editDay ? m_model.editHumidPct : m_model.targetHumidPct,
                      day == m_model.editDay ? (m_model.editTurning ? "ON" : "OFF") : "-",
                      day == m_model.editDay ? m_model.editIntervalMin : 0U);
        m_display.setTextSize(2);
        m_display.setTextColor(sel ? Color::kText : Color::kTextDim, bg);
        m_display.drawText(14, y + 6, row);
    }
}

void MainUiRenderer::renderPlanEdit()
{
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "Day %u", m_model.editDay);
    drawRow(54, "일차", buffer, false, kTeal);
    std::snprintf(buffer, sizeof(buffer), "%.1f C", m_model.editTempC);
    drawRow(84, "온도", buffer, m_model.fieldCursor == 0, Color::kAccentTemp);
    std::snprintf(buffer, sizeof(buffer), "%.0f %%", m_model.editHumidPct);
    drawRow(114, "습도", buffer, m_model.fieldCursor == 1, Color::kAccentHumi);
    drawRow(144, "전란", m_model.editTurning ? "ON" : "OFF", m_model.fieldCursor == 2,
            m_model.editTurning ? kOk : Color::kOffIcon);
    std::snprintf(buffer, sizeof(buffer), "%u min", m_model.editIntervalMin);
    drawRow(174, "간격", buffer, m_model.fieldCursor == 3, kTeal);

    uint32_t saveBg = (m_model.fieldCursor == 4U) ? Color::kSelected : kPanel;
    uint32_t cancelBg = (m_model.fieldCursor == 5U) ? Color::kSelected : kPanel;
    m_display.fillRect(68, 198, 78, 16, saveBg);
    m_display.drawRect(68, 198, 78, 16, (m_model.fieldCursor == 4U) ? kOk : kPanelSoft);
    m_display.fillRect(174, 198, 78, 16, cancelBg);
    m_display.drawRect(174, 198, 78, 16, (m_model.fieldCursor == 5U) ? Color::kOffIcon : kPanelSoft);
    m_display.setTextSize(1);
    m_display.setTextColor(Color::kText, saveBg);
    m_display.drawText(90, 201, "저장");
    m_display.setTextColor(Color::kText, cancelBg);
    m_display.drawText(196, 201, "취소");

    if (m_model.editMode && m_model.fieldCursor <= 3U) {
        m_display.drawRect(10, 82 + static_cast<int>(m_model.fieldCursor) * 30, 300, 30, kWarn);
    }
}

void MainUiRenderer::renderManual()
{
    drawRow(62, "히터", m_model.heaterOn ? "ON" : "OFF", m_model.manualCursor == 0,
            m_model.heaterOn ? kOk : Color::kOffIcon);
    drawRow(96, "가습기", m_model.humidifierOn ? "ON" : "OFF", m_model.manualCursor == 1,
            m_model.humidifierOn ? kOk : Color::kOffIcon);
    drawRow(130, "전란", m_model.turnerOn ? "ON" : "OFF", m_model.manualCursor == 2,
            m_model.turnerOn ? kOk : Color::kOffIcon);
    drawRow(164, "팬", m_model.fanOn ? "ON" : "OFF", m_model.manualCursor == 3,
            m_model.fanOn ? kOk : Color::kOffIcon);
}

void MainUiRenderer::renderConfirm(const char*, const char* line1, const char* line2)
{
    m_display.setTextSize(1);
    m_display.setTextColor(Color::kText, Color::kBg);
    m_display.drawText(34, 82, line1);
    m_display.setTextColor(Color::kTextDim, Color::kBg);
    m_display.drawText(34, 104, line2);
    drawRow(148, "아니오", "취소", m_model.confirmCursor == 0, Color::kOffIcon);
    drawRow(180, "예", "실행", m_model.confirmCursor == 1, kDanger);
}

void MainUiRenderer::renderFactoryReset()
{
    m_display.setTextSize(1);
    m_display.setTextColor(kDanger, Color::kBg);
    m_display.drawText(24, 76, "모든 설정, WiFi, 부화 계획이 삭제됩니다.");
    m_display.setTextColor(Color::kTextDim, Color::kBg);
    m_display.drawText(24, 100, "실행하려면 버튼을 10초간 계속 누르세요.");
    drawProgressBar(32, 138, 256, 18, m_model.factoryProgressPct, kDanger);
    char pct[16];
    std::snprintf(pct, sizeof(pct), "%u%%", m_model.factoryProgressPct);
    m_display.setTextColor(Color::kText, Color::kBg);
    m_display.drawText(144, 166, pct);
}

void MainUiRenderer::drawStatusIcons()
{
    m_display.setTextSize(1);
    drawPill(8, 218, 42, "히터", m_model.heaterOn ? kOk : Color::kOffIcon);
    drawPill(54, 218, 42, "가습", m_model.humidifierOn ? kOk : Color::kOffIcon);
    drawPill(100, 218, 42, "전란", m_model.turnerOn ? kOk : Color::kOffIcon);
    drawPill(146, 218, 32, "팬", m_model.fanOn ? kOk : Color::kOffIcon);
}

void MainUiRenderer::drawSignalBars(int x, int y, bool connected)
{
    uint32_t color = connected ? kOk : Color::kOffIcon;
    m_display.fillRect(x, y + 9, 3, 3, color);
    m_display.fillRect(x + 5, y + 6, 3, 6, color);
    m_display.fillRect(x + 10, y + 3, 3, 9, color);
}

void MainUiRenderer::drawProgressBar(int x, int y, int w, int h, uint8_t pct, uint32_t color)
{
    if (pct > 100U) pct = 100U;
    int fillW = (w - 4) * static_cast<int>(pct) / 100;
    m_display.drawRect(x, y, w, h, kPanelSoft);
    m_display.fillRect(x + 2, y + 2, w - 4, h - 4, 0x0000U);
    if (fillW > 0) m_display.fillRect(x + 2, y + 2, fillW, h - 4, color);
}

void MainUiRenderer::drawPill(int x, int y, int w, const char* label, uint32_t color)
{
    m_display.drawRect(x, y, w, 20, color);
    m_display.setTextSize(1);
    m_display.setTextColor(Color::kText, Color::kBg);
    m_display.drawText(x + (hasUtf8(label) ? 8 : 6), y + 4, label);
}

void MainUiRenderer::drawRow(int y, const char* label, const char* value, bool selected, uint32_t accent)
{
    uint32_t bg = selected ? Color::kSelected : kPanel;
    if (accent == 0) accent = Color::kTextDim;
    m_display.fillRect(12, y, 296, 28, bg);
    m_display.drawRect(12, y, 296, 28, selected ? accent : kPanelSoft);
    m_display.fillRect(18, y + 7, 6, 14, accent);
    m_display.setTextSize(1);
    m_display.setTextColor(Color::kTextDim, bg);
    m_display.drawText(32, y + 5, label);
    bool compact = hasUtf8(value) || std::strlen(value) > 9U;
    m_display.setTextSize(compact ? 1 : 2);
    m_display.setTextColor(Color::kText, bg);
    m_display.drawText(compact ? 178 : 208, compact ? y + 10 : y + 7, value);
}

} // namespace incubator::ui
