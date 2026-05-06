
# DDU-POWER-001 — Power & Watchdog Pipeline

> Version: 1.0
> Status: Draft
> Target: ESP32-S3

핵심 흐름:

Power Monitor
    ↓
Watchdog
    ↓
RecoveryManager
    ↓
SafeMode

목표:

- Brownout Detect
- WDT Recovery
- PowerFail Recovery
- Safe Shutdown

---

# 생성 파일

product/power/PowerState.h
product/power/PowerMonitor.h
product/power/PowerMonitor.cpp
product/power/WatchdogManager.h
product/power/WatchdogManager.cpp

---

# 핵심 철학

전원이 불안정하면
출력보다 안전이 우선이다.

---

# PowerState.h

```cpp
#pragma once

namespace incubator::power
{
    struct PowerState
    {
        bool brownoutDetected = false;

        bool watchdogTriggered = false;

        bool powerHealthy = true;

        float inputVoltage = 0.0f;
    };
}
```

---

# 목표

- Brownout Detect
- Safe Shutdown
- WDT Recovery
- RecoveryManager 연계
- SafeMode 보호

---

# 다음 단계

DDU-MEM-001
Memory & Performance Pipeline
