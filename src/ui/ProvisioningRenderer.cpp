#include "ui/ProvisioningRenderer.h"
#include "ui/UiColors.h"
#include <cstdio>

namespace incubator::ui
{

void ProvisioningRenderer::render(uint32_t)
{
    char payload[160];
    std::snprintf(payload, sizeof(payload),
                  "{\"ver\":\"v1\",\"name\":\"%s\",\"pop\":\"%s\",\"transport\":\"ble\"}",
                  m_model.provisioningName,
                  m_model.provisioningPop);

    m_display.fillRect(0, 0, 320, 240, Color::kBg);
    m_display.fillRect(40, 0, 240, 240, 0xFFFFU);
    m_display.drawQrCode(payload, 44, 4, 232, 6, true);
}

} // namespace incubator::ui
