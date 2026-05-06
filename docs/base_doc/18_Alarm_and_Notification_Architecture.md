# 18_Alarm_and_Notification_Architecture

> Version: 1.0
> Status: Authoritative

---

# 목적

본 문서는 Alarm 및 Notification 구조를 정의한다.

---

# 1. Alarm 철학

```text
Alarm은 즉시 인지 가능해야 한다.
```

---

# 2. Alarm 종류

| Alarm | 설명 |
|---|---|
| HighTemp | 고온 |
| LowTemp | 저온 |
| HighHumidity | 고습 |
| LowHumidity | 저습 |
| SensorFail | 센서 오류 |
| PlanCorrupt | 계획 손상 |

---

# 3. Alarm 구조

```cpp
enum class AlarmType
{
    None,
    HighTemp,
    LowTemp,
    SensorFail
};
```

---

# 4. Alarm 상태 흐름

```text
RuntimeState
    ↓
AlarmDetector
    ↓
AlarmEvent
    ↓
UI Overlay
    ↓
Cloud Publish
```

---

# 5. Notification 계층

| Level | UX |
|---|---|
| Info | Toast |
| Warning | Yellow Banner |
| Alarm | Red Overlay |
| Critical | SafeMode |

---

# 6. Alarm Overlay 원칙

```text
현재 작업보다 우선해야 한다.
```

---

# 7. 최종 원칙

```text
Alarm은 로그가 아니라
사용자 행동을 유도해야 한다.
```
