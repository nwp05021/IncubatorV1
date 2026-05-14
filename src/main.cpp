#include <Arduino.h>
#include "Application.h"

// 전역 또는 정적 메모리 영역에 단일 애플리더 인스턴스를 격리 생성합니다.
static incubator::Application app;

void setup()
{
    // 하드웨어 제어 및 내부 컴포넌트 전체 연쇄 가동
    app.setup();
}

void loop()
{
    // 무한 루프 스케줄링 갱신
    app.loop();
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
