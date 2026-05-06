
# DDU-IMPL-007 — AppSettings Actual Source

> Version: 1.0
> Status: Draft
> Target: ESP32-S3 + PlatformIO

---

# 1. 목적

Persistent Settings 구조 실제 구현.

---

# 2. 핵심 철학

```text
Settings는
사용자 정책이다.
```

---

# 3. AppSettings.h

```cpp
#pragma once

#include <stdint.h>

namespace incubator::domain
{
    struct AppSettings
    {
        // ---------- Climate ----------

        float tempHysteresis = 0.3f;

        float humidityHysteresis = 3.0f;

        // ---------- Fan ----------

        uint8_t fanNormalPwm = 40;

        uint8_t fanLockdownPwm = 60;

        // ---------- Turning ----------

        uint32_t turningIntervalMs =
            7200000;

        // ---------- Cloud ----------

        uint32_t telemetryIntervalMs =
            60000;

        // ---------- UI ----------

        bool ambientUiEnabled = true;
    };
}
```

---

# 4. RuntimeState와 차이

| 항목 | RuntimeState | AppSettings |
|---|---|---|
| 현재 온도 | O | X |
| 목표 온도 | O | X |
| hysteresis | X | O |
| telemetry interval | X | O |

---

# 5. 핵심 원칙

```text
Settings는 자주 변하지 않는다.
```

---

# 6. 저장 규칙

반드시:

```text
Command
    ↓
AppController
    ↓
Storage Save
```

---

# 7. 금지 사항

```text
❌ Runtime 값 저장

❌ Device pointer 저장

❌ RuntimeState direct reference 보관
```

---

# 8. Acceptance Criteria

```text
AC-1
기본값 안전

AC-2
Heap 사용 없음

AC-3
Persistent 정책 분리

AC-4
UI/Cloud 공용 사용 가능

AC-5
Validation 가능 구조 유지
```
