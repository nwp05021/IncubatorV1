# 16_RuntimeState_Reference

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 RuntimeState의 표준 구조를 정의한다.

---

# 1. 핵심 철학

```text
RuntimeState는
현재 시스템 상태의 유일한 진실이다.
```

---

# 2. RuntimeState 역할

| 역할 | 설명 |
|---|---|
| UI 표시 | 현재 상태 |
| Cloud Report | reported |
| Telemetry | 발행 |
| Alarm 판단 | 현재 상태 |
| Debug | 상태 추적 |

---

# 3. 권장 구조

```cpp
struct RuntimeState
{
    // Sensor
    float currentTempC;
    float currentHumidityPct;

    // Targets
    float targetTempC;
    float targetHumidityPct;

    // Outputs
    bool heaterOn;
    bool humidifierOn;
    bool turnerOn;

    // Batch
    uint16_t currentDay;
    bool lockdown;

    // System
    bool safeMode;
    bool wifiConnected;
    bool awsConnected;

    // Alarm
    bool highTempAlarm;
    bool lowTempAlarm;
};
```

---

# 4. 금지

```text
❌ UI direct mutation
❌ Cloud direct mutation
❌ Device mutation
```

---

# 5. 최종 원칙

```text
읽기는 어디서나 가능하지만
쓰기는 중앙화한다.
```
