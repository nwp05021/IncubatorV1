# 17_AppSettings_and_Config_Model

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 AppSettings 및 시스템 설정 구조를 정의한다.

---

# 1. 역할

정적 운영 설정 저장.

---

# 2. 저장 위치

```text
NVS
```

---

# 3. 권장 구조

```cpp
struct AppSettings
{
    float tempHysteresis;
    float humidityHysteresis;

    uint32_t telemetryIntervalMs;

    bool cloudEnabled;

    bool alarmEnabled;

    uint16_t turningDurationSec;
};
```

---

# 4. 변경 원칙

설정 변경은 반드시:

```text
Command
    ↓
AppController
```

흐름 사용.

---

# 5. Validation 예시

| 항목 | 범위 |
|---|---|
| Temp Hysteresis | 0.1 ~ 2.0 |
| Humidity Hysteresis | 1 ~ 10 |
| Telemetry Interval | >= 10 sec |

---

# 6. 최종 원칙

```text
설정은 안정성을 우선한다.
```
