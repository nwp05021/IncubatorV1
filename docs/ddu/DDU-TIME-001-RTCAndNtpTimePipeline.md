
# DDU-TIME-001 — RTC + NTP Time Pipeline

> Version: 1.0
> Status: Draft
> Target: ESP32-S3
> Dependency:
> - RuntimeState
> - WifiManager
> - Scheduler

---

# 목적

Offline First 기반 Time Pipeline 구현.

핵심 흐름:

RTC
    ↓
TimeService
    ↓
RuntimeState
    ↓
Scheduler / Cloud

목표:

- RTC Restore
- NTP Sync
- Offline Time 유지
- Epoch Tracking
- Time Service 중앙화

---

# 생성 파일

product/time/TimeState.h
product/time/TimeService.h
product/time/TimeService.cpp

---

# 핵심 철학

WiFi가 없어도 시간은 유지되어야 한다.

---

# TimeState.h

```cpp
#pragma once

#include <stdint.h>

namespace incubator::time
{
    struct TimeState
    {
        bool rtcValid = false;

        bool ntpSynced = false;

        uint32_t currentEpoch = 0;

        uint32_t lastNtpSyncEpoch = 0;
    };
}
```

---

# TimeService.h

```cpp
#pragma once

#include "TimeState.h"

namespace incubator::time
{
    class TimeService
    {
    public:
        TimeService(
            TimeState& state);

    public:
        void begin();

        void tick(uint32_t nowMs);

        uint32_t nowEpoch() const;

    private:
        void restoreRtc();

        void syncNtp();

    private:
        TimeState& m_state;

        uint32_t m_lastTickMs = 0;

        uint32_t m_lastNtpSyncMs = 0;

        static constexpr uint32_t TickIntervalMs =
            1000;

        static constexpr uint32_t NtpSyncIntervalMs =
            3600000;
    };
}
```

---

# 핵심 전략

RTC Restore
NTP Sync
Offline Continue
Epoch Tracking

---

# 다음 단계

DDU-POWER-001
Power & Watchdog Pipeline
