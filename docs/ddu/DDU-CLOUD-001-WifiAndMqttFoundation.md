
# DDU-CLOUD-001 — WiFi + MQTT Foundation

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + AWS IoT
> Dependency:
> - RuntimeState
> - AppController
> - CommandQueue
>
> Estimated Time: 20~40 min

---

# 1. 목적

Offline First 기반 Cloud Foundation 구현.

핵심 흐름:

```text
RuntimeState
    ↓
TelemetryBuilder
    ↓
AwsIotClient
    ↓
AWS IoT

AWS desired
    ↓
CmdParser
    ↓
Command
    ↓
AppController
```

목표:

- WiFi Recovery
- MQTT Retry
- Offline First
- Cloud State 분리
- Command 기반 Cloud

---

# 2. 생성 파일

```text
product/cloud/WifiManager.h
product/cloud/WifiManager.cpp

product/cloud/AwsIotClient.h
product/cloud/AwsIotClient.cpp

product/cloud/CloudState.h
```

---

# 3. 핵심 철학

```text
Cloud는 제품을 확장하는 기능이다.
```

---

## 가장 중요한 원칙

```text
WiFi가 죽어도
제품은 살아야 한다.
```

---

# 4. CloudState

## 역할

Cloud 연결 상태 관리.

---

## CloudState.h

```cpp
#pragma once

namespace incubator::cloud
{
    struct CloudState
    {
        bool wifiConnected = false;

        bool mqttConnected = false;

        bool shadowSubscribed = false;

        bool telemetryEnabled = true;
    };
}
```

---

# 5. WifiManager

## 역할

```text
WiFi 연결
Retry
상태 갱신
```

---

## 금지

```text
❌ MQTT 처리

❌ RuntimeState 직접 수정

❌ GPIO 접근
```

---

## WifiManager.h

```cpp
#pragma once

#include "CloudState.h"

namespace incubator::cloud
{
    class WifiManager
    {
    public:
        WifiManager(
            CloudState& state);

    public:
        void begin();

        void tick(uint32_t nowMs);

    private:
        void connect();

    private:
        CloudState& m_state;

        uint32_t m_lastRetryMs = 0;

        static constexpr uint32_t RetryIntervalMs = 30000;
    };
}
```

---

# 6. AwsIotClient

## 역할

```text
MQTT Connect
Publish
Subscribe
Retry
```

---

## AwsIotClient.h

```cpp
#pragma once

#include "CloudState.h"

namespace incubator::cloud
{
    class AwsIotClient
    {
    public:
        AwsIotClient(
            CloudState& state);

    public:
        void begin();

        void tick(uint32_t nowMs);

        bool publish(
            const char* topic,
            const char* payload);

    private:
        void reconnect();

    private:
        CloudState& m_state;

        uint32_t m_lastRetryMs = 0;

        static constexpr uint32_t RetryIntervalMs = 15000;
    };
}
```

---

# 7. Offline First 전략

## 핵심 원칙

```text
Cloud는 선택 기능
```

---

## 허용

```text
WiFi Disconnect
MQTT Disconnect
AWS 장애
```

---

## 금지

```text
Cloud 장애로 Climate 중단
```

---

# 8. MQTT Retry 전략

| 항목 | 정책 |
|---|---|
| WiFi Retry | 30 sec |
| MQTT Retry | 15 sec |
| TLS Fail | Backoff |
| DNS Fail | Retry |

---

# 9. Main Loop 연결

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
}
```

---

# 10. RuntimeState 연계

## 원칙

```text
Cloud는 RuntimeState 읽기 전용
```

---

## Reported

```text
RuntimeState
    ↓
Telemetry
```

---

## Desired

```text
AWS desired
    ↓
Command
```

---

# 11. SafeMode 연계

## reported

```text
safeMode = true
```

즉시 Cloud 동기화.

---

# 12. 핵심 장점

## 1) Cloud 독립성

Cloud 장애와 제품 분리.

---

## 2) RuntimeState 중심 구조

UI/Cloud 동일 데이터 사용.

---

## 3) Retry 중앙화

WiFi/MQTT 정책 단순화.

---

# 13. 금지 사항

```text
❌ Cloud direct GPIO

❌ Cloud direct RuntimeState mutation

❌ MQTT blocking wait

❌ delay()
```

---

# 14. Acceptance Criteria

```text
AC-1
WiFi Retry 정상 동작

AC-2
MQTT Retry 정상 동작

AC-3
Offline 상태에서도 제품 정상 동작

AC-4
CloudState 정상 갱신

AC-5
Main Loop blocking 없음
```

---

# 15. 다음 단계

다음 DDU:

```text
DDU-CLOUD-002
TelemetryBuilder + ShadowSync
```

다음 구현 예정:

- RuntimeState → JSON
- Shadow reported
- Dirty Telemetry
- Offline Queue
- Delta Sync
