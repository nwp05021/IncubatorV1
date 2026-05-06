
# DDU-CLOUD-002 — TelemetryBuilder + ShadowSync

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + AWS IoT
> Dependency:
> - RuntimeState
> - AwsIotClient
> - CloudState
>
> Estimated Time: 20~40 min

---

# 1. 목적

RuntimeState 기반 Telemetry 및 Shadow Sync 구현.

핵심 흐름:

```text
RuntimeState
    ↓
TelemetryBuilder
    ↓
JSON
    ↓
MQTT Publish
```

목표:

- Shadow reported
- Dirty Sync
- Telemetry Rate Control
- Offline Queue
- JSON 고정 구조

---

# 2. 생성 파일

```text
product/cloud/TelemetryPayload.h

product/cloud/TelemetryBuilder.h
product/cloud/TelemetryBuilder.cpp

product/cloud/ShadowSyncManager.h
product/cloud/ShadowSyncManager.cpp
```

---

# 3. 핵심 철학

```text
Telemetry는 현재 상태이지
내부 구현 세부사항이 아니다.
```

---

# 4. TelemetryPayload

## 목적

Cloud Publish 데이터 구조.

---

## TelemetryPayload.h

```cpp
#pragma once

namespace incubator::cloud
{
    struct TelemetryPayload
    {
        float currentTempC = 0.0f;

        float currentHumidityPct = 0.0f;

        float targetTempC = 0.0f;

        float targetHumidityPct = 0.0f;

        bool heaterOn = false;

        bool humidifierOn = false;

        bool safeMode = false;

        uint16_t currentDay = 0;

        bool wifiConnected = false;

        bool awsConnected = false;
    };
}
```

---

# 5. TelemetryBuilder

## 역할

```text
RuntimeState
    ↓
TelemetryPayload
    ↓
JSON
```

---

## 금지

```text
❌ MQTT Publish 직접 수행

❌ RuntimeState 직접 수정
```

---

## TelemetryBuilder.h

```cpp
#pragma once

#include "../domain/RuntimeState.h"
#include "TelemetryPayload.h"

namespace incubator::cloud
{
    class TelemetryBuilder
    {
    public:
        void build(
            const incubator::domain::RuntimeState& runtime,
            TelemetryPayload& payload);

        bool serialize(
            const TelemetryPayload& payload,
            char* buffer,
            size_t size);
    };
}
```

---

# 6. TelemetryBuilder.cpp

```cpp
#include "TelemetryBuilder.h"

namespace incubator::cloud
{
    using namespace incubator::domain;

    void TelemetryBuilder::build(
        const RuntimeState& runtime,
        TelemetryPayload& payload)
    {
        payload.currentTempC =
            runtime.currentTempC;

        payload.currentHumidityPct =
            runtime.currentHumidityPct;

        payload.targetTempC =
            runtime.targetTempC;

        payload.targetHumidityPct =
            runtime.targetHumidityPct;

        payload.heaterOn =
            runtime.heaterOn;

        payload.humidifierOn =
            runtime.humidifierOn;

        payload.safeMode =
            runtime.safeMode;

        payload.currentDay =
            runtime.currentDay;

        payload.wifiConnected =
            runtime.wifiConnected;

        payload.awsConnected =
            runtime.awsConnected;
    }

    bool TelemetryBuilder::serialize(
        const TelemetryPayload& payload,
        char* buffer,
        size_t size)
    {
        // TODO:
        // StaticJsonDocument Serialize

        return true;
    }
}
```

---

# 7. ShadowSyncManager

## 역할

```text
Dirty Check
Publish Timing
Offline Queue
```

---

## ShadowSyncManager.h

```cpp
#pragma once

#include "CloudState.h"
#include "AwsIotClient.h"
#include "TelemetryBuilder.h"

namespace incubator::cloud
{
    class ShadowSyncManager
    {
    public:
        ShadowSyncManager(
            CloudState& state,
            AwsIotClient& aws,
            TelemetryBuilder& builder);

    public:
        void tick(uint32_t nowMs);

    private:
        uint32_t m_lastPublishMs = 0;

        static constexpr uint32_t PublishIntervalMs =
            60000;

    private:
        CloudState& m_state;

        AwsIotClient& m_aws;

        TelemetryBuilder& m_builder;
    };
}
```

---

# 8. Dirty Sync 전략

## 핵심 원칙

```text
안 보내는 것이 더 중요하다.
```

---

## Publish 조건

```text
값 변경 발생
OR
60초 경과
```

---

# 9. Offline Queue 전략

## 목적

WiFi 단절 시 Telemetry 손실 감소.

---

## 구조

```text
Fixed RingBuffer
```

---

## 정책

```text
FIFO
고정 크기
오래된 데이터 Drop
```

---

# 10. JSON 구조 예시

```json
{
  "tempC": 37.5,
  "humidityPct": 60.0,
  "heater": true,
  "safeMode": false,
  "day": 7
}
```

---

# 11. Main Loop 연결

## main.cpp

```cpp
void loop()
{
    const uint32_t now = millis();

    sensorManager.tick(now);

    scheduler.tick(now);

    climate.tick(now);

    alarm.tick(now);

    recovery.tick(now);

    appController.tick();

    ui.tick(now);

    wifi.tick(now);

    aws.tick(now);

    shadow.tick(now);
}
```

---

# 12. RuntimeState 연계

## 핵심 구조

```text
RuntimeState
    ↓
TelemetryBuilder
    ↓
JSON
```

---

## 금지

```text
❌ Cloud 내부 상태 계산
```

---

# 13. SafeMode 전략

## 정책

```text
SafeMode 발생 시
즉시 Publish
```

---

## 이유

```text
사용자 즉시 인지
```

---

# 14. 핵심 장점

## 1) Cloud 비용 감소

Dirty Sync 기반.

---

## 2) RuntimeState 중심 구조

UI/Cloud 동일 데이터 사용.

---

## 3) Offline First 유지

Queue 기반 Publish.

---

# 15. 금지 사항

```text
❌ Dynamic JSON growth

❌ Runtime malloc 반복

❌ MQTT blocking publish

❌ Cloud direct GPIO
```

---

# 16. Acceptance Criteria

```text
AC-1
TelemetryPayload 정상 생성

AC-2
JSON Serialize 정상 동작

AC-3
Dirty Sync 정상 동작

AC-4
Offline Queue 정상 동작

AC-5
60초 Publish 정상 동작
```

---

# 17. 다음 단계

다음 DDU:

```text
DDU-CLOUD-003
Cloud Command Parser
```

다음 구현 예정:

- desired JSON
- Command Parsing
- Delta Sync
- Command Validation
- Queue Injection
