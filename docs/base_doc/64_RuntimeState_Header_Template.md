# 64_RuntimeState_Header_Template

> Version: 1.0
> Status: Implementation Template

---

# 목적

본 문서는 RuntimeState 실제 구현용 템플릿을 제공한다.

---

# 권장 구조

```cpp
#pragma once

namespace incubator::domain
{
    struct RuntimeState
    {
        // Sensor
        float currentTempC = 0.0f;
        float currentHumidityPct = 0.0f;

        // Target
        float targetTempC = 37.8f;
        float targetHumidityPct = 60.0f;

        // Output
        bool heaterOn = false;
        bool humidifierOn = false;
        bool fanOn = false;
        bool turnerOn = false;

        // Batch
        bool batchActive = false;
        uint16_t currentDay = 0;

        // Network
        bool wifiConnected = false;
        bool awsConnected = false;

        // Alarm
        bool highTempAlarm = false;
        bool lowTempAlarm = false;

        // Safety
        bool safeMode = false;
    };
}
```

---

# 핵심 원칙

```text
읽기는 자유롭게,
쓰기는 AppController만.
```
